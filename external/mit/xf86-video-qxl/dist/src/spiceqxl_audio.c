/*
 * Copyright 2012 Andrew Eikum for CodeWeavers Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* XSpice based audio feed; reads from files (presumably fifos) in a configured directory,
   and mixes their raw data on to the spice playback channel.  */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "spiceqxl_audio.h"

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>
#if defined(HAVE_SYS_INOTIFY_H)
#include <sys/inotify.h>
#endif

/* mplayer + pulse will write data to the fifo as fast as we can read it.
       So we need to pace both how quickly we consume the data and how quickly
       we feed the data in to Spice.  We will read ahead (up to READ_BUFFER_PERIODS),
       and feed ahead into the Spice server (up to FEED_BUFFER_PERIODS).
*/

#define IDLE_MS              300
#define PERIOD_MS            10
#define READ_BUFFER_PERIODS  2
#define FEED_BUFFER_PERIODS  8

#define MAX_FIFOS 16

struct fifo_data {
    char *buffer;
    int   size;
    int   len;
    int   add_to;
    int   fd;
    SpiceWatch *watch;
};

struct audio_data {
    struct fifo_data fifos[MAX_FIFOS];
    int active;
    uint32_t *spice_buffer;
    int spice_buffer_bytes;
    int period_bytes;
    struct timeval fed_through_time;
    int remainder;
    int fifo_count;
    int closed_fifos;
    SpiceTimer *wall_timer;
    int wall_timer_type;
    int dir_watch;
    int fifo_dir_watch;
    SpiceWatch *fifo_dir_qxl_watch;
};

/* We maintain a ring buffer for each file we are reading from;
   these helper functions facilitate adding data to the buffer,
   and removing it.  */
static inline void fifo_data_added(struct fifo_data *f, int n)
{
    f->add_to = (f->add_to + n) % f->size;
    f->len += n;
}

static inline int fifo_read(struct fifo_data *f)
{
    int rc;
    int len = min(f->size - f->len, f->size - f->add_to);
    rc = read(f->fd, f->buffer + f->add_to, len);
    if (rc > 0)
        fifo_data_added(f, rc);

    if (rc > 0 && rc == len && f->size - f->len > 0) {
        rc = read(f->fd, f->buffer + f->add_to, f->size - f->len);
        if (rc > 0)
            fifo_data_added(f, rc);
    }

    return rc;
}

static inline void fifo_remove_data(struct fifo_data *f, unsigned char *dest, int len)
{
    int remove_from = f->add_to >= f->len ? f->add_to - f->len : f->add_to + f->size - f->len;
    int remain = f->size - remove_from;

    if (remain < len) {
        if (dest) {
            memcpy(dest, f->buffer + remove_from, remain);
            dest += remain;
        }
        len -= remain;
        f->len -= remain;
        remove_from = 0;
    }

    if (dest) {
        memcpy(dest, f->buffer + remove_from, len);
    }
    f->len -= len;
}

static void mix_in_one_fifo(struct fifo_data *f, int16_t *out, int len)
{
    int s;
    int16_t *in;

    if (len > f->len)
        len = f->len;

    in = calloc(1, len);

    fifo_remove_data(f, (unsigned char *) in, len);

    for (s = 0; s < (len / sizeof(int16_t)); s++) {
        /* FIXME: Ehhh, this'd be better as floats. With this algorithm,
         * samples mixed after being clipped will have undue weight. But
         * if we're clipping, then we're distorted anyway, so whatever. */
        if (out[s] + in[s] > INT16_MAX)
            out[s] = INT16_MAX;
        else if (out[s] + in[s] < -INT16_MAX)
            out[s] = -INT16_MAX;
        else
            out[s] += in[s];
    }

    free(in);
}

/* a helper for process_fifos() */
static void mix_in_fifos(qxl_screen_t *qxl)
{
    int i;
    struct audio_data *data = qxl->playback_opaque;
    struct fifo_data *f;

    if (data->spice_buffer) {
        memset(data->spice_buffer, 0, data->spice_buffer_bytes);
    }

    if (data->fifo_count == 0)
        return;

    /* First fifo can just be copied */
    f = &data->fifos[0];
    fifo_remove_data(f, (unsigned char *) data->spice_buffer, min(data->spice_buffer_bytes, f->len));

    /* Extra fifos need to be mixed in */
    for (i = 1; i < data->fifo_count; i++) {
        f = &data->fifos[i];
        if (f->len > 0) {
            if (data->spice_buffer) {
                mix_in_one_fifo(f, (int16_t *) data->spice_buffer, data->spice_buffer_bytes);
            } else {
                fifo_remove_data(f, NULL, min(data->spice_buffer_bytes, f->len));
            }
        }
    }
}

/* a helper for process_fifos() */
static int can_feed(struct audio_data *data)
{
    struct timeval end, diff;

    gettimeofday(&end, NULL);

    if (end.tv_sec > data->fed_through_time.tv_sec ||
        (end.tv_sec == data->fed_through_time.tv_sec &&
         end.tv_usec >= data->fed_through_time.tv_usec)) {
        data->fed_through_time.tv_sec = data->fed_through_time.tv_usec = 0;
        data->remainder = 0;
        return 1;
    }

    timersub(&data->fed_through_time, &end, &diff);
    if (diff.tv_sec == 0 && diff.tv_usec < PERIOD_MS * 1000 * FEED_BUFFER_PERIODS)
        return 1;

    return 0;
}

/* a helper for process_fifos() */
static void did_feed(struct audio_data *data, int len)
{
    struct timeval diff;

    if (data->fed_through_time.tv_sec == 0 && data->fed_through_time.tv_usec == 0)
        gettimeofday(&data->fed_through_time, NULL);

    diff.tv_sec = 0;
    diff.tv_usec = (data->remainder + (len * PERIOD_MS * 1000)) / data->period_bytes;
    data->remainder = (data->remainder + (len * PERIOD_MS * 1000)) % data->period_bytes;

    timeradd(&data->fed_through_time, &diff, &data->fed_through_time);
}

static int process_fifos(qxl_screen_t *qxl, struct audio_data *data, int maxlen)
{
    while (maxlen > 0) {
        if (! data->spice_buffer) {
            uint32_t chunk_frames;
            spice_server_playback_get_buffer(&qxl->playback_sin, &data->spice_buffer, &chunk_frames);
            data->spice_buffer_bytes = data->spice_buffer ?
                chunk_frames * sizeof(int16_t) * SPICE_INTERFACE_PLAYBACK_CHAN :
                data->period_bytes * READ_BUFFER_PERIODS;
        }

        if (! can_feed(data)) {
            return FALSE;
        }

        mix_in_fifos(qxl);

        did_feed(data, data->spice_buffer_bytes);
        maxlen -= data->spice_buffer_bytes;

        if (data->spice_buffer) {
            spice_server_playback_put_samples(&qxl->playback_sin, data->spice_buffer);
            data->spice_buffer = NULL;
        }
    }
    return TRUE;
}

/* a helper for read_from_fifos() */
static void condense_fifos(qxl_screen_t *qxl)
{
    struct audio_data *data = qxl->playback_opaque;
    int i;

    for (i = 0; i < data->fifo_count; i++) {
        struct fifo_data *f = &data->fifos[i];
        if (f->fd == -1 && f->len == 0) {
            if ((i + 1) < data->fifo_count) {
                struct fifo_data tmp = *f;
                *f = data->fifos[data->fifo_count - 1];
                data->fifos[data->fifo_count - 1] = tmp;
            }
            data->fifo_count--;
            i--;
            if (!--data->closed_fifos) {
                break;
            }
        }
    }
}

static void start_watching(qxl_screen_t *qxl);
static void read_from_fifos(int fd, int event, void *opaque)
{
    qxl_screen_t *qxl = opaque;
    struct audio_data *data = qxl->playback_opaque;
    int i;
    int maxlen = 0;

    if (data->wall_timer_type) {
        qxl->core->timer_cancel(data->wall_timer);
        data->wall_timer_type = 0;
    }

    for (i = 0; i < data->fifo_count; i++) {
        struct fifo_data *f = &data->fifos[i];

        if (f->size - f->len > 0 && f->fd >= 0) {
            int rc;

            rc = fifo_read(f);
            if (rc == -1 && (errno == EAGAIN || errno == EINTR))
                /* no new data to read */;
            else if (rc <= 0) {
                if (rc == 0)
                    ErrorF("fifo %d closed\n", f->fd);
                else
                    ErrorF("fifo %d error %d: %s\n", f->fd, errno, strerror(errno));

                if (f->watch)
                    qxl->core->watch_remove(f->watch);
                f->watch = NULL;
                close(f->fd);
                f->fd = -1;
                /* Setting closed_fifos will only have an effect once
                 * the closed fifo's buffer is empty.
                 */
                data->closed_fifos++;
            }

            if (f->size == f->len) {
                if (f->watch)
                    qxl->core->watch_remove(f->watch);
                f->watch = NULL;
            }
        }

        if (f->len > maxlen)
            maxlen = f->len;
    }

    if (data->closed_fifos) {
        condense_fifos(qxl);
    }

    if (maxlen && !data->active) {
        spice_server_playback_start(&qxl->playback_sin);
        data->active = 1;
    }

    if (!process_fifos(qxl, data, maxlen)) {
        /* There is still some fifo data to process */
        qxl->core->timer_start(data->wall_timer, PERIOD_MS);
        data->wall_timer_type = PERIOD_MS;

    } else if (data->fifo_count) {
        /* All the fifo data was processed. Wait for more */
        start_watching(qxl);

        /* But none may arrive so stop processing if that happens */
        qxl->core->timer_start(data->wall_timer, IDLE_MS);
        data->wall_timer_type = IDLE_MS;

    } else if (data->active) {
        /* There is no open fifo anymore */
        spice_server_playback_stop(&qxl->playback_sin);
        data->active = 0;
    }
}

/* a helper for read_from_fifos() */
static void start_watching(qxl_screen_t *qxl)
{
    struct audio_data *data = qxl->playback_opaque;
    int i;

    for (i = 0; i < data->fifo_count; i++) {
        struct fifo_data *f = &data->fifos[i];
        if (f->watch || f->size == f->len || f->fd == -1)
            continue;

        f->watch = qxl->core->watch_add(f->fd, SPICE_WATCH_EVENT_READ, read_from_fifos, qxl);
    }
}

/* a helper for read_from_fifos() */
static void wall_ticker(void *opaque)
{
    qxl_screen_t *qxl = opaque;
    struct audio_data *data = qxl->playback_opaque;

    if (data->wall_timer_type == IDLE_MS) {
        /* The audio is likely paused in the application(s) */
        if (data->active) {
            spice_server_playback_stop(&qxl->playback_sin);
            data->active = 0;
        }
        data->wall_timer_type = 0;
    } else {
        data->wall_timer_type = 0;
        read_from_fifos(-1, 0, qxl);
    }
}

#if defined(HAVE_SYS_INOTIFY_H)
static void handle_one_change(qxl_screen_t *qxl, struct inotify_event *e)
{

    if (e->mask & (IN_CREATE | IN_MOVED_TO)) {
        struct audio_data *data = qxl->playback_opaque;
        struct fifo_data *f;
        char *fname;

        f = &data->fifos[data->fifo_count];

        if (data->fifo_count == MAX_FIFOS) {
            static int once = 0;
            if (!once) {
                ErrorF("playback: Too many FIFOs already open\n");
                ++once;
            }
            return;
        }

        fname = xnfalloc(strlen(e->name) + strlen(qxl->playback_fifo_dir) + 1 + 1);
        strcpy(fname, qxl->playback_fifo_dir);
        strcat(fname, "/");
        strcat(fname, e->name);

        f->fd = open(fname, O_RDONLY | O_RSYNC | O_NONBLOCK);
        free(fname);
        if (f->fd < 0) {
            ErrorF("playback: open FIFO '%s' failed: %s\n", e->name, strerror(errno));
            return;
        }

        ErrorF("playback: opened FIFO '%s' as %d:%d\n", e->name, data->fifo_count, f->fd);

        data->fifo_count++;

        f->watch = qxl->core->watch_add(f->fd, SPICE_WATCH_EVENT_READ, read_from_fifos, qxl);
    }
}

static void playback_dir_changed(int fd, int event, void *opaque)
{
    qxl_screen_t *qxl = opaque;
    static unsigned char buf[sizeof(struct inotify_event) + NAME_MAX + 1];
    static int offset = 0;
    struct inotify_event *e;
    int rc;

    do {
        rc = read(fd, buf + offset, sizeof(buf) - offset);
        if (rc > 0) {
            offset += rc;
            if (offset >= sizeof(*e)) {
                int len;
                e = (struct inotify_event *) buf;
                len = sizeof(*e) + e->len;
                if (offset >= len) {
                    handle_one_change(qxl, e);
                    if (offset > len)
                        memmove(buf, buf + offset, offset - len);
                    offset -= len;
                }
            }
        }
    }
    while (rc > 0);
}
#endif



static const SpicePlaybackInterface playback_sif = {
    {
        SPICE_INTERFACE_PLAYBACK,
        "playback",
        SPICE_INTERFACE_PLAYBACK_MAJOR,
        SPICE_INTERFACE_PLAYBACK_MINOR
    }
};

static void audio_initialize (qxl_screen_t *qxl)
{
    int i;
    struct audio_data *data = qxl->playback_opaque;
    int freq = SPICE_INTERFACE_PLAYBACK_FREQ;
    int period_frames;
    int frame_bytes;

#if SPICE_INTERFACE_PLAYBACK_MAJOR > 1 || SPICE_INTERFACE_PLAYBACK_MINOR >= 3
    freq = spice_server_get_best_playback_rate(&qxl->playback_sin);
#endif

    period_frames = freq * PERIOD_MS / 1000;
    frame_bytes = sizeof(int16_t) * SPICE_INTERFACE_PLAYBACK_CHAN;
    data->period_bytes = period_frames * frame_bytes;

    for (i = 0; i < MAX_FIFOS; ++i) {
        data->fifos[i].fd = -1;
        data->fifos[i].size = data->period_bytes * READ_BUFFER_PERIODS;
        data->fifos[i].buffer = calloc(1, data->fifos[i].size);
    }
}


int
qxl_add_spice_playback_interface (qxl_screen_t *qxl)
{
    int ret;
    struct audio_data *data = calloc(1, sizeof(*data));

#if defined(HAVE_SYS_INOTIFY_H) && defined(HAVE_INOTIFY_INIT1)
    if (qxl->playback_fifo_dir[0] == 0) {
        ErrorF("playback: no audio FIFO directory, audio is disabled\n");
        free(data);
        return 0;
    }

    qxl->playback_sin.base.sif = &playback_sif.base;
    ret = spice_server_add_interface(qxl->spice_server, &qxl->playback_sin.base);
    if (ret < 0) {
        free(data);
        return errno;
    }

#if SPICE_INTERFACE_PLAYBACK_MAJOR > 1 || SPICE_INTERFACE_PLAYBACK_MINOR >= 3
    spice_server_set_playback_rate(&qxl->playback_sin,
            spice_server_get_best_playback_rate(&qxl->playback_sin));
#else
    /* disable CELT */
    ret = spice_server_set_playback_compression(qxl->spice_server, 0);
    if (ret < 0) {
        free(data);
        return errno;
    }
#endif


    qxl->playback_opaque = data;
    audio_initialize(qxl);

    data->wall_timer = qxl->core->timer_add(wall_ticker, qxl);

    data->dir_watch = inotify_init1(IN_NONBLOCK);
    data->fifo_dir_watch = -1;
    if (data->dir_watch >= 0)
        data->fifo_dir_watch = inotify_add_watch(data->dir_watch, qxl->playback_fifo_dir, IN_CREATE | IN_MOVE);

    if (data->fifo_dir_watch == -1) {
        ErrorF("Error %s(%d) watching the fifo dir\n", strerror(errno), errno);
        return errno;
    }

    data->fifo_dir_qxl_watch = qxl->core->watch_add(data->dir_watch,
            SPICE_WATCH_EVENT_READ, playback_dir_changed, qxl);

#else
    ErrorF("inotify not available; audio disabled.\n");
#endif
    return 0;
}
