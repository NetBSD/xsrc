/*
 * Copyright 2011 Red Hat, Inc.
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

/** \file spiceqxl_spice_server.c
 * \author Alon Levy <alevy@redhat.com>
 *
 * spice server helpers for spiceqxl.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "qxl.h"
#include "qxl_option_helpers.h"
#include "spiceqxl_spice_server.h"

/* Single instance of spice server per Xorg executable.
 */
SpiceServer *xspice_get_spice_server(void)
{
    static SpiceServer *spice_server;
    if (!spice_server) {
        spice_server = spice_server_new();
    }
    return spice_server;
}

#define X509_CA_CERT_FILE "ca-cert.pem"
#define X509_SERVER_KEY_FILE "server-key.pem"
#define X509_SERVER_CERT_FILE "server-cert.pem"

/* config string parsing */

#define SPICE_ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static int name2enum(const char *string, const char *table[], int entries)
{
    int i;

    if (string) {
        for (i = 0; i < entries; i++) {
            if (!table[i]) {
                continue;
            }
            if (strcmp(string, table[i]) != 0) {
                continue;
            }
            return i;
        }
    }
    return -1;
}

static int parse_name(const char *string, const char *optname,
                      const char *table[], int entries)
{
    int value = name2enum(string, table, entries);

    if (value != -1) {
        return value;
    }
    fprintf(stderr, "spice: invalid %s: %s\n", optname, string);
    exit(1);
}

static const char *stream_video_names[] = {
    [ SPICE_STREAM_VIDEO_OFF ]    = "off",
    [ SPICE_STREAM_VIDEO_ALL ]    = "all",
    [ SPICE_STREAM_VIDEO_FILTER ] = "filter",
};
#define parse_stream_video(_name) \
    name2enum(_name, stream_video_names, SPICE_ARRAY_SIZE(stream_video_names))

static const char *compression_names[] = {
    [ SPICE_IMAGE_COMPRESS_OFF ]      = "off",
    [ SPICE_IMAGE_COMPRESS_AUTO_GLZ ] = "auto_glz",
    [ SPICE_IMAGE_COMPRESS_AUTO_LZ ]  = "auto_lz",
    [ SPICE_IMAGE_COMPRESS_QUIC ]     = "quic",
    [ SPICE_IMAGE_COMPRESS_GLZ ]      = "glz",
    [ SPICE_IMAGE_COMPRESS_LZ ]       = "lz",
};
#define parse_compression(_name)                                        \
    parse_name(_name, "image compression",                              \
               compression_names, SPICE_ARRAY_SIZE(compression_names))

static const char *wan_compression_names[] = {
    [ SPICE_WAN_COMPRESSION_AUTO   ] = "auto",
    [ SPICE_WAN_COMPRESSION_NEVER  ] = "never",
    [ SPICE_WAN_COMPRESSION_ALWAYS ] = "always",
};
#define parse_wan_compression(_name)                                    \
    parse_name(_name, "wan compression",                                \
               wan_compression_names, SPICE_ARRAY_SIZE(wan_compression_names))

void xspice_set_spice_server_options(OptionInfoPtr options)
{
    /* environment variables take precedence. If not then take
     * parameters from the config file. */
    int addr_flags;
    int len;
    spice_image_compression_t compression;
    spice_wan_compression_t wan_compr;
    int port = get_int_option(options, OPTION_SPICE_PORT, "XSPICE_PORT");
    int tls_port =
        get_int_option(options, OPTION_SPICE_TLS_PORT, "XSPICE_TLS_PORT");
    const char *password =
        get_str_option(options, OPTION_SPICE_PASSWORD, "XSPICE_PASSWORD");
    int disable_ticketing =
        get_bool_option(options, OPTION_SPICE_DISABLE_TICKETING, "XSPICE_DISABLE_TICKETING");
    const char *x509_dir =
        get_str_option(options, OPTION_SPICE_X509_DIR, "XSPICE_X509_DIR");
    int sasl = get_bool_option(options, OPTION_SPICE_SASL, "XSPICE_SASL");
    const char *x509_key_file_base =
        get_str_option(options, OPTION_SPICE_X509_KEY_FILE,
                       "XSPICE_X509_KEY_FILE");
    char *x509_key_file = NULL;
    const char *x509_cert_file_base =
        get_str_option(options, OPTION_SPICE_X509_CERT_FILE,
                       "XSPICE_X509_CERT_FILE");
    char *x509_cert_file = NULL;
    const char *x509_key_password =
        get_str_option(options, OPTION_SPICE_X509_KEY_PASSWORD,
                    "XSPICE_X509_KEY_PASSWORD");
    const char *tls_ciphers =
        get_str_option(options, OPTION_SPICE_TLS_CIPHERS,
                    "XSPICE_TLS_CIPHERS");
    const char *x509_cacert_file_base =
        get_str_option(options, OPTION_SPICE_CACERT_FILE,
                    "XSPICE_CACERT_FILE");
    char *x509_cacert_file = NULL;
    const char * addr =
        get_str_option(options, OPTION_SPICE_ADDR, "XSPICE_ADDR");
    int ipv4 =
        get_bool_option(options, OPTION_SPICE_IPV4_ONLY, "XSPICE_IPV4_ONLY");
    int ipv6 =
        get_bool_option(options, OPTION_SPICE_IPV6_ONLY, "XSPICE_IPV6_ONLY");
    const char *x509_dh_file =
        get_str_option(options, OPTION_SPICE_DH_FILE, "XSPICE_DH_FILE");
    int disable_copy_paste =
        get_bool_option(options, OPTION_SPICE_DISABLE_COPY_PASTE,
                        "XSPICE_DISABLE_COPY_PASTE");
    int exit_on_disconnect =
        get_bool_option(options, OPTION_SPICE_EXIT_ON_DISCONNECT,
                        "XSPICE_EXIT_ON_DISCONNECT");
    const char *image_compression =
        get_str_option(options, OPTION_SPICE_IMAGE_COMPRESSION,
                       "XSPICE_IMAGE_COMPRESSION");
    const char *jpeg_wan_compression =
        get_str_option(options, OPTION_SPICE_JPEG_WAN_COMPRESSION,
                       "XSPICE_JPEG_WAN_COMPRESSION");
    const char *zlib_glz_wan_compression =
        get_str_option(options, OPTION_SPICE_ZLIB_GLZ_WAN_COMPRESSION,
                       "XSPICE_ZLIB_GLZ_WAN_COMPRESSION");
    const char *streaming_video =
        get_str_option(options, OPTION_SPICE_STREAMING_VIDEO,
                       "XSPICE_STREAMING_VIDEO");
    const char *video_codecs =
        get_str_option(options, OPTION_SPICE_VIDEO_CODECS,
                       "XSPICE_VIDEO_CODECS");
    int agent_mouse =
        get_bool_option(options, OPTION_SPICE_AGENT_MOUSE,
                        "XSPICE_AGENT_MOUSE");
    int playback_compression =
        get_bool_option(options, OPTION_SPICE_PLAYBACK_COMPRESSION,
                        "XSPICE_PLAYBACK_COMPRESSION");

    SpiceServer *spice_server = xspice_get_spice_server();

    if (!port && !tls_port) {
        printf("one of tls-port or port must be set\n");
        exit(1);
    }
    printf("xspice: port = %d, tls_port = %d\n", port, tls_port);
    if (disable_ticketing) {
        spice_server_set_noauth(spice_server);
    }
    if (tls_port) {
        if (NULL == x509_dir) {
            x509_dir = ".";
        }
        len = strlen(x509_dir) + 32;

        if (x509_key_file_base) {
            x509_key_file = xnfstrdup(x509_key_file_base);
        } else {
            x509_key_file = xnfalloc(len);
            snprintf(x509_key_file, len, "%s/%s", x509_dir, X509_SERVER_KEY_FILE);
        }

        if (x509_cert_file_base) {
            x509_cert_file = xnfstrdup(x509_cert_file_base);
        } else {
            x509_cert_file = xnfalloc(len);
            snprintf(x509_cert_file, len, "%s/%s", x509_dir, X509_SERVER_CERT_FILE);
        }

        if (x509_cacert_file_base) {
            x509_cacert_file = xnfstrdup(x509_cert_file_base);
        } else {
            x509_cacert_file = xnfalloc(len);
            snprintf(x509_cacert_file, len, "%s/%s", x509_dir, X509_CA_CERT_FILE);
        }
    }

    addr_flags = 0;
    if (ipv4) {
        addr_flags |= SPICE_ADDR_FLAG_IPV4_ONLY;
    } else if (ipv6) {
        addr_flags |= SPICE_ADDR_FLAG_IPV6_ONLY;
    }

    spice_server_set_addr(spice_server, addr ? addr : "", addr_flags);
    if (port) {
        spice_server_set_port(spice_server, port);
    }
    if (tls_port) {
        spice_server_set_tls(spice_server, tls_port,
                             x509_cacert_file,
                             x509_cert_file,
                             x509_key_file,
                             x509_key_password,
                             x509_dh_file,
                             tls_ciphers);
    }
    if (password) {
        spice_server_set_ticket(spice_server, password, 0, 0, 0);
    }
    if (sasl) {
#if SPICE_SERVER_VERSION >= 0x000802 /* 0.8.2 */
        if (spice_server_set_sasl_appname(spice_server, "xspice") == -1 ||
            spice_server_set_sasl(spice_server, 1) == -1) {
            fprintf(stderr, "spice: failed to enable sasl\n");
            exit(1);
        }
#else
        fprintf(stderr, "spice: sasl is not available (spice >= 0.8.2 required)\n");
        exit(1);
#endif
    }

#if SPICE_SERVER_VERSION >= 0x000801
    /* we still don't actually support agent in xspice, but this
     * can't hurt for later, just copied from qemn/ui/spice-core.c */
    if (disable_copy_paste) {
        spice_server_set_agent_copypaste(spice_server, 0);
    }
#endif

    if (exit_on_disconnect) {
#if SPICE_SERVER_VERSION >= 0x000b04 /* 0.11.4 */
        spice_server_set_exit_on_disconnect(spice_server, exit_on_disconnect);
#else
        fprintf(stderr, "spice: cannot set exit_on_disconnect (spice >= 0.11.4 required)\n");
        exit(1);
#endif
    }

    compression = SPICE_IMAGE_COMPRESS_AUTO_GLZ;
    if (image_compression) {
        compression = parse_compression(image_compression);
    }
    spice_server_set_image_compression(spice_server, compression);

    wan_compr = SPICE_WAN_COMPRESSION_AUTO;
    if (jpeg_wan_compression) {
        wan_compr = parse_wan_compression(jpeg_wan_compression);
    }
    spice_server_set_jpeg_compression(spice_server, wan_compr);

    wan_compr = SPICE_WAN_COMPRESSION_AUTO;
    if (zlib_glz_wan_compression) {
        wan_compr = parse_wan_compression(zlib_glz_wan_compression);
    }
    spice_server_set_zlib_glz_compression(spice_server, wan_compr);

    if (streaming_video) {
        int streaming_video_opt = parse_stream_video(streaming_video);
        spice_server_set_streaming_video(spice_server, streaming_video_opt);
    }

    if (video_codecs) {
#if SPICE_SERVER_VERSION >= 0x000d02 /* 0.13.2 */
        if (spice_server_set_video_codecs(spice_server, video_codecs)) {
            fprintf(stderr, "spice: invalid video encoder %s\n", video_codecs);
            exit(1);
        }
#else
        fprintf(stderr, "spice: video_codecs are not available (spice >= 0.13.2 required)\n");
        exit(1);
#endif
    }

    spice_server_set_agent_mouse(spice_server, agent_mouse);
    spice_server_set_playback_compression(spice_server, playback_compression);

    free(x509_key_file);
    free(x509_cert_file);
    free(x509_cacert_file);
}
