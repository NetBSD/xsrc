/*
 * Copyright (C) 2014 CodeWeavers, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Jeremy White <jwhite@codeweavers.com>
 */

/*----------------------------------------------------------------------------
  Chip/Smart Card Interface Devices driver for Spice

    This driver is built to interface to pcsc-lite as a serial smartcard
  device.
    It translates the IFD (Interface device) ABI into the Spice protocol.
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "vscard_common.h"
#include "ifdhandler.h"
#include <arpa/inet.h>

typedef struct apdu_list {
    void *data;
    int len;
    struct apdu_list *next;
} apdu_t;

#define MAX_LUNS    2
typedef struct smartcard_ccid {
    int fd;
    int lun;
    pthread_t tid;
    int state;
    char atr[36];
    int  atr_len;
    pthread_mutex_t apdu_lock;
    apdu_t *apdu_list;
} smartcard_ccid_t;

#define STATE_OPEN                  1
#define STATE_READER_ADDED          2
#define STATE_READER_REMOVED        4

#if ! defined(MIN)
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif


smartcard_ccid_t luns[MAX_LUNS] = { { -1 }, { -1 } };

RESPONSECODE IFDHCloseChannel(DWORD Lun);

static void push_apdu(smartcard_ccid_t *ccid, void *data, int len)
{
    apdu_t *a = malloc(sizeof(*a));
    apdu_t **p;

    a->data = malloc(len);
    a->len = len;
    a->next = NULL;
    memcpy(a->data, data, len);

    pthread_mutex_lock(&ccid->apdu_lock);
    for (p = &ccid->apdu_list; *p; p = &(*p)->next)
        ;
    *p = a;

    pthread_mutex_unlock(&ccid->apdu_lock);
}

static apdu_t * pop_apdu(smartcard_ccid_t *ccid)
{
    apdu_t *p;
    pthread_mutex_lock(&ccid->apdu_lock);
    p = ccid->apdu_list;
    if (ccid->apdu_list)
        ccid->apdu_list = p->next;
    pthread_mutex_unlock(&ccid->apdu_lock);
    return p;
}

static void free_apdu(apdu_t *a)
{
    free(a->data);
    free(a);
}

static void send_reply(smartcard_ccid_t *ccid, uint32_t code)
{
    uint32_t reply[4];

    reply[0] = htonl(VSC_Error);        // type
    reply[1] = htonl(ccid->lun);        // reader id
    reply[2] = htonl(sizeof(uint32_t)); // length
    reply[3] = htonl(code);             // Error code

    if (write(ccid->fd, (char *) reply, sizeof(reply)) != sizeof(reply)) {
        fprintf(stderr, "Error: lun %d fd %d write failed; errno %d\n", ccid->lun, ccid->fd, errno);
        IFDHCloseChannel(ccid->lun);
    }
}

static int send_tx_buffer(smartcard_ccid_t *ccid, void *data, int len)
{
    uint32_t *reply, *p;
    int write_len = sizeof(*reply) * 3 + len;

    reply = malloc(write_len);
    p = reply;

    *p++ = htonl(VSC_APDU);         // type
    *p++ = htonl(ccid->lun);        // reader id
    *p++ = htonl(len);
    memcpy(p, data, len);

    if (write(ccid->fd, (char *) reply, write_len) != write_len) {
        fprintf(stderr, "Error: lun %d fd %d write failed; errno %d\n", ccid->lun, ccid->fd, errno);
        IFDHCloseChannel(ccid->lun);
        free(reply);
        return 0;
    }
    free(reply);
    return 1;
}

static void process_reader_add(smartcard_ccid_t *ccid, VSCMsgHeader *h, char *data)
{
    if (ccid->state & STATE_READER_ADDED) {
        send_reply(ccid, VSC_GENERAL_ERROR);
        return;
    }

    ccid->state |= STATE_READER_ADDED;
    ccid->state &= ~STATE_READER_REMOVED;

    pthread_mutex_init(&ccid->apdu_lock, NULL);
    ccid->apdu_list = NULL;

    send_reply(ccid, VSC_SUCCESS);
}

static void process_reader_remove(smartcard_ccid_t *ccid, VSCMsgHeader *h)
{
    apdu_t *p;

    if (ccid->state & STATE_READER_REMOVED) {
        send_reply(ccid, VSC_GENERAL_ERROR);
        return;
    }

    ccid->state |= STATE_READER_REMOVED;
    ccid->state &= ~STATE_READER_ADDED;

    while (p = pop_apdu(ccid))
        free_apdu(p);

    pthread_mutex_destroy(&ccid->apdu_lock);

    send_reply(ccid, VSC_SUCCESS);
}

static void process_atr(smartcard_ccid_t *ccid, VSCMsgHeader *h, char *data)
{
    ccid->atr_len = h->length;
    if (h->length > sizeof(ccid->atr)) {
        fprintf(stderr, "Supplied ATR of length %d exceeds %d maximum\n",
            h->length, sizeof(ccid->atr));
        send_reply(ccid, VSC_GENERAL_ERROR);
        return;
    }

    memset(ccid->atr, 0, sizeof(ccid->atr));
    memcpy(ccid->atr, data, ccid->atr_len);

    send_reply(ccid, VSC_SUCCESS);
}

static void process_apdu(smartcard_ccid_t *ccid, VSCMsgHeader *h, char *data)
{
    if (ccid->state & STATE_READER_ADDED)
        push_apdu(ccid, data, h->length);
    else
        fprintf(stderr, "apdu of length %d discarded; inactive reader\n", h->length);
}

static void process_card_remove(smartcard_ccid_t *ccid, VSCMsgHeader *h)
{
    ccid->atr_len = 0;
    memset(ccid->atr, 0, sizeof(ccid->atr));
    send_reply(ccid, VSC_SUCCESS);
}

static int process_message(smartcard_ccid_t *ccid, char *buf, int len)
{
    VSCMsgHeader h;
    uint32_t *p = (uint32_t *) buf;

    h.type = ntohl(*p++);
    h.reader_id = ntohl(*p++);
    h.length = ntohl(*p++);

    if (len < sizeof(h) || len < sizeof(h) + h.length)
        return 0;

    switch (h.type) {
        case VSC_ReaderAdd:
            process_reader_add(ccid, &h, h.length > 0 ? buf + sizeof(h) : NULL);
            break;

        case VSC_ReaderRemove:
            process_reader_remove(ccid, &h);
            break;

        case VSC_ATR:
            process_atr(ccid, &h, h.length > 0 ? buf + sizeof(h) : NULL);
            break;

        case VSC_CardRemove:
            process_card_remove(ccid, &h);
            break;

        case VSC_APDU:
            process_apdu(ccid, &h, h.length > 0 ? buf + sizeof(h) : NULL);
            break;

        default:
            fprintf(stderr, "spiceccid %s: unknown smartcard message %d / %d\n", __FUNCTION__, h.type, sizeof(h) + h.length);

    }

    return(h.length + sizeof(h));
}

static void * lun_thread(void *arg)
{
    char buf[8096];
    int pos = 0;
    smartcard_ccid_t *ccid = (smartcard_ccid_t *) arg;
    int rc;

    while (1) {
        rc = read(ccid->fd, buf + pos, sizeof(buf) - pos);
        if (rc == -1)
            if (errno == EINTR)
                continue;
            else
                break;

        if (rc == 0)
            break;

        pos += rc;

        do {
            rc = process_message(ccid, buf, pos);
            pos -= rc;

            if (rc > 0 && pos > 0)
                memmove(buf, buf + rc, pos);
        } while (rc > 0 && pos > 0);
    }

    fprintf(stderr, "LUN %d thread exiting: %s\n", ccid->lun,
            rc == 0 ? "normally" : strerror(errno));
    close(ccid->fd);
    ccid->fd = -1;
    ccid->lun = 0;
    ccid->atr_len = 0;
    ccid->state &= ~STATE_OPEN;

    return NULL;
}


static void send_init(smartcard_ccid_t *ccid)
{
    uint32_t msg[6];

    msg[0] = htonl(VSC_Init);               // type
    msg[1] = htonl(ccid->lun);              // reader id
    msg[2] = htonl(sizeof(uint32_t) * 3);   // length
    msg[3] = htonl(VSCARD_MAGIC);           // VSCD
    msg[4] = htonl(VSCARD_VERSION);         // VSCD
    msg[5] = 0;                             // capabilities

    if (write(ccid->fd, (char *) msg, sizeof(msg)) != sizeof(msg)) {
        fprintf(stderr, "Error: lun %d fd %d write failed; errno %d\n", ccid->lun, ccid->fd, errno);
        IFDHCloseChannel(ccid->lun);
    }
}

/*----------------------------------------------------------------------------
    IFDHCreateChannelByName
        The pcsc daemon should invoke this function passing in the path name
    configured in reader.conf.
*/
RESPONSECODE IFDHCreateChannelByName(DWORD Lun, LPSTR DeviceName)
{
    int i;
    struct sockaddr_un addr;

    for (i = 0; i < MAX_LUNS; i++)
        if (luns[i].fd != -1 && luns[i].lun == Lun)
            return IFD_COMMUNICATION_ERROR;

    for (i = 0; i < MAX_LUNS; i++)
        if (luns[i].fd == -1)
            break;

    if (i >= MAX_LUNS)
        return IFD_COMMUNICATION_ERROR;

    luns[i].fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (luns[i].fd < 0)
        return IFD_NO_SUCH_DEVICE;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, DeviceName, sizeof(addr.sun_path) - 1);
    if (connect(luns[i].fd, (struct sockaddr *) &addr, sizeof(addr))) {
        close(luns[i].fd);
        return IFD_COMMUNICATION_ERROR;
    }

    if (pthread_create(&luns[i].tid, NULL, &lun_thread, &luns[i])) {
        close(luns[i].fd);
        return IFD_COMMUNICATION_ERROR;
    }

    luns[i].lun = Lun;
    luns[i].state = STATE_OPEN;

    return IFD_SUCCESS;
}

RESPONSECODE IFDHCreateChannel(DWORD Lun, DWORD Channel)
{
    fprintf(stderr, "spiceccid %s unsupported: Lun %ld, Channel %ld\n", __FUNCTION__, Lun, Channel);
    return IFD_ERROR_NOT_SUPPORTED;
}

RESPONSECODE IFDHCloseChannel(DWORD Lun)
{
    int i;

    for (i = 0; i < MAX_LUNS; i++) {
        if (luns[i].fd != -1 && luns[i].lun == Lun) {
            pthread_cancel(luns[i].tid);
            close(luns[i].fd);
            luns[i].fd = -1;
            luns[i].lun = 0;
            luns[i].atr_len = 0;
            luns[i].state &= ~STATE_OPEN;
            break;
        }
    }

    if (i == MAX_LUNS)
        return IFD_NO_SUCH_DEVICE;

    return IFD_SUCCESS;
}

RESPONSECODE IFDHGetCapabilities(DWORD Lun, DWORD Tag, PDWORD Length, PUCHAR Value)
{
    fprintf(stderr, "spiceccid %s unsupported: Lun %ld, Tag %ld, Length %ld, Value %p\n", __FUNCTION__, Lun, Tag, *Length, Value);
    /* TODO - explore supporting TAG_IFD_POLLING_THREAD */
    return IFD_ERROR_NOT_SUPPORTED;
}

RESPONSECODE IFDHSetCapabilities(DWORD Lun, DWORD Tag, DWORD Length, PUCHAR Value)
{
    return IFD_ERROR_NOT_SUPPORTED;
}

RESPONSECODE IFDHPowerICC(DWORD Lun, DWORD Action, PUCHAR Atr, PDWORD AtrLength)
{
    int i;

    for (i = 0; i < MAX_LUNS; i++)
        if (luns[i].fd != -1 && luns[i].lun == Lun)
            if (Action == IFD_POWER_UP || Action == IFD_RESET) {
                if (*AtrLength >= luns[i].atr_len) {
                    memcpy(Atr, luns[i].atr, luns[i].atr_len);
                    *AtrLength = luns[i].atr_len;
                }
                send_init(&luns[i]);
                return IFD_SUCCESS;
            }

    fprintf(stderr, "spiceccid %s unsupported: Lun %ld, Action %ld\n", __FUNCTION__, Lun, Action);
    return IFD_ERROR_NOT_SUPPORTED;
}

#define TX_MAX_SLEEP 5000
#define TX_SLEEP_INTERVAL 1000
RESPONSECODE IFDHTransmitToICC(DWORD Lun, SCARD_IO_HEADER SendPci,
    PUCHAR TxBuffer, DWORD TxLength, PUCHAR RxBuffer, PDWORD
    RxLength, PSCARD_IO_HEADER RecvPci)
{
    apdu_t *p;
    int i, j;

    for (i = 0; i < MAX_LUNS; i++)
        if (luns[i].fd != -1 && luns[i].lun == Lun) {
            while (p = pop_apdu(&luns[i]))
                free_apdu(p);

            if (send_tx_buffer(&luns[i], TxBuffer, TxLength)) {
                for (j = 0; j < TX_MAX_SLEEP; j++)
                    if (p = pop_apdu(&luns[i]))
                        break;
                    else
                        usleep(TX_SLEEP_INTERVAL);

                if (p) {
                    memcpy(RxBuffer, p->data, MIN(p->len, *RxLength));
                    *RxLength = MIN(p->len, *RxLength);
                    free_apdu(p);
                    return IFD_SUCCESS;
                }

                return IFD_RESPONSE_TIMEOUT;
            }
        }
    return IFD_NO_SUCH_DEVICE;
}

RESPONSECODE IFDHICCPresence(DWORD Lun)
{
    int i;

    for (i = 0; i < MAX_LUNS; i++)
        if (luns[i].fd != -1 && luns[i].lun == Lun) {
            if (luns[i].atr_len > 0 && luns[i].state & STATE_READER_ADDED)
                return IFD_SUCCESS;

            return IFD_ICC_NOT_PRESENT;
        }

    return IFD_NO_SUCH_DEVICE;
}

RESPONSECODE IFDHSetProtocolParameters(DWORD Lun, DWORD Protocol, UCHAR Flags,
    UCHAR PTS1, UCHAR PTS2, UCHAR PTS3)
{
    if (Protocol == SCARD_PROTOCOL_T1)
        return IFD_SUCCESS;

    return IFD_NOT_SUPPORTED;
}

RESPONSECODE IFDHControl(DWORD Lun, DWORD dwControlCode, PUCHAR
    TxBuffer, DWORD TxLength, PUCHAR RxBuffer, DWORD RxLength,
    LPDWORD pdwBytesReturned)
{
    fprintf(stderr, "spiceccid %s unsupported: Lun %ld\n", __FUNCTION__, Lun);
    return IFD_ERROR_NOT_SUPPORTED;
}
