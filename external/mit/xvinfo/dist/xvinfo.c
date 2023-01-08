#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xvlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char *progname;

static void _X_NORETURN _X_COLD
PrintUsage(void)
{
    fprintf(stderr, "Usage: %s [-display host:dpy] [-short] [-version]\n",
            progname);
    exit(0);
}

int
main(int argc, char *argv[])
{
    Display *dpy;
    unsigned int ver, rev, eventB, reqB, errorB;
    int nscreens;
    char *disname = NULL;
    char shortmode = 0;

    progname = argv[0];

    if ((argc > 4))
        PrintUsage();

    if (argc != 1) {
        for (int i = 1; i < argc; i++) {
            if (!strcmp(argv[i], "-display")) {
                if (++i >= argc) {
                    fprintf (stderr, "%s: missing argument to -display\n",
                             progname);
                    PrintUsage();
                }
                disname = argv[i];
            }
            else if (!strcmp(argv[i], "-short"))
                shortmode = 1;
            else if (!strcmp(argv[i], "-version")) {
                printf("%s\n", PACKAGE_STRING);
                exit(0);
            }
            else {
                fprintf (stderr, "%s: unrecognized argument '%s'\n",
                         progname, argv[i]);
                PrintUsage();
            }
        }
    }

    if (!(dpy = XOpenDisplay(disname))) {
        fprintf(stderr, "%s:  Unable to open display %s\n", progname,
                (disname != NULL) ? disname : XDisplayName(NULL));
        exit(-1);
    }

    if (Success != XvQueryExtension(dpy, &ver, &rev, &reqB, &eventB, &errorB)) {
        fprintf(stderr, "%s: No X-Video Extension on %s\n", progname,
                (disname != NULL) ? disname : XDisplayName(NULL));
        exit(0);
    }
    else {
        fprintf(stdout, "X-Video Extension version %i.%i\n", ver, rev);
    }

    nscreens = ScreenCount(dpy);

    for (int i = 0; i < nscreens; i++) {
        unsigned int nadaptors;
        XvAdaptorInfo *ainfo;

        fprintf(stdout, "screen #%i\n", i);
        if (Success != XvQueryAdaptors(dpy, RootWindow(dpy, i), &nadaptors,
                                       &ainfo)) {
            fprintf(stderr, "%s:  Failed to query adaptors on display %s\n",
                    progname, (disname != NULL) ? disname : XDisplayName(NULL));
            exit(-1);
        }

        if (!nadaptors) {
            fprintf(stdout, " no adaptors present\n");
            continue;
        }

        for (unsigned int j = 0; j < nadaptors; j++) {
            XvFormat *format;
            int  nattr;
            XvAttribute *attributes;
            unsigned int nencode;
            XvEncodingInfo *encodings;

            fprintf(stdout, "  Adaptor #%i: \"%s\"\n", j, ainfo[j].name);
            fprintf(stdout, "    number of ports: %li\n", ainfo[j].num_ports);
            fprintf(stdout, "    port base: %li\n", ainfo[j].base_id);
            fprintf(stdout, "    operations supported: ");
            switch (ainfo[j].type & (XvInputMask | XvOutputMask)) {
            case XvInputMask:
                if (ainfo[j].type & XvVideoMask)
                    fprintf(stdout, "PutVideo ");
                if (ainfo[j].type & XvStillMask)
                    fprintf(stdout, "PutStill ");
                if (ainfo[j].type & XvImageMask)
                    fprintf(stdout, "PutImage ");
                break;
            case XvOutputMask:
                if (ainfo[j].type & XvVideoMask)
                    fprintf(stdout, "GetVideo ");
                if (ainfo[j].type & XvStillMask)
                    fprintf(stdout, "GetStill ");
                break;
            default:
                fprintf(stdout, "none ");
                break;
            }
            fprintf(stdout, "\n");

            format = ainfo[j].formats;

            if (!shortmode) {
                fprintf(stdout, "    supported visuals:\n");
                for (unsigned long k = 0; k < ainfo[j].num_formats; k++, format++) {
                    fprintf(stdout, "      depth %i, visualID 0x%2lx\n",
                            format->depth, format->visual_id);
                }
            }

            attributes = XvQueryPortAttributes(dpy, ainfo[j].base_id, &nattr);

            if (attributes && nattr) {
                fprintf(stdout, "    number of attributes: %i\n", nattr);

                for (int k = 0; k < nattr; k++) {
                    fprintf(stdout, "      \"%s\" (range %i to %i)\n",
                            attributes[k].name,
                            attributes[k].min_value, attributes[k].max_value);

                    if (attributes[k].flags & XvSettable) {
                        if (!shortmode)
                            fprintf(stdout,
                                    "              client settable attribute\n");
                        else
                            fprintf(stdout, "              settable");
                    }

                    if (attributes[k].flags & XvGettable) {
                        Atom the_atom;

                        int value;

                        if (!shortmode)
                            fprintf(stdout,
                                    "              client gettable attribute");
                        else
                            fprintf(stdout, ", gettable");

                        the_atom = XInternAtom(dpy, attributes[k].name, True);

                        if (the_atom != None) {
                            if ((Success == XvGetPortAttribute(dpy,
                                                               ainfo[j].base_id,
                                                               the_atom,
                                                               &value)))
                                fprintf(stdout, " (current value is %i)",
                                        value);
                        }
                        fprintf(stdout, "\n");
                    }
                    else if (shortmode)
                        fprintf(stdout, "\n");

                }
                XFree(attributes);
            }
            else {
                fprintf(stdout, "    no port attributes defined\n");
            }

            if (Success != XvQueryEncodings(dpy, ainfo[j].base_id, &nencode,
                                            &encodings)) {
                fprintf(stderr,
                        "%s:  Failed to query encodings on display %s\n",
                        progname,
                        (disname != NULL) ? disname : XDisplayName(NULL));
                exit(-1);
            }

            if (encodings && nencode) {
                unsigned int ImageEncodings = 0;

                for (unsigned int n = 0; n < nencode; n++) {
                    if (!strcmp(encodings[n].name, "XV_IMAGE"))
                        ImageEncodings++;
                }

                if (nencode - ImageEncodings) {
                    fprintf(stdout, "    number of encodings: %i\n",
                            nencode - ImageEncodings);

                    for (unsigned int n = 0; n < nencode; n++) {
                        if (strcmp(encodings[n].name, "XV_IMAGE")) {
                            fprintf(stdout, "      encoding ID #%li: \"%s\"\n",
                                    encodings[n].encoding_id,
                                    encodings[n].name);
                            fprintf(stdout, "        size: %li x %li\n",
                                    encodings[n].width, encodings[n].height);
                            fprintf(stdout, "        rate: %f\n",
                                    (double) encodings[n].rate.numerator /
                                    (double) encodings[n].rate.denominator);
                        }
                    }
                }

                if (ImageEncodings && (ainfo[j].type & XvImageMask)) {
                    int numImages;
                    XvImageFormatValues *formats;

                    for (unsigned int n = 0; n < nencode; n++) {
                        if (!strcmp(encodings[n].name, "XV_IMAGE")) {
                            fprintf(stdout,
                                    "    maximum XvImage size: %li x %li\n",
                                    encodings[n].width, encodings[n].height);
                            break;
                        }
                    }

                    formats =
                        XvListImageFormats(dpy, ainfo[j].base_id, &numImages);

                    fprintf(stdout, "    Number of image formats: %i\n",
                            numImages);

                    for (int n = 0; n < numImages; n++) {
                        char imageName[5];

                        snprintf(imageName, sizeof(imageName), "%c%c%c%c",
                                 formats[n].id & 0xff,
                                (formats[n].id >> 8) & 0xff,
                                (formats[n].id >> 16) & 0xff,
                                (formats[n].id >> 24) & 0xff);
                        fprintf(stdout, "      id: 0x%x", formats[n].id);
                        if (isprint(imageName[0]) && isprint(imageName[1]) &&
                            isprint(imageName[2]) && isprint(imageName[3])) {
                            fprintf(stdout, " (%s)\n", imageName);
                        }
                        else {
                            fprintf(stdout, "\n");
                        }
                        if (!shortmode) {
                            fprintf(stdout, "        guid: ");
                            fprintf(stdout, "%02x", (unsigned char)
                                    formats[n].guid[0]);
                            fprintf(stdout, "%02x", (unsigned char)
                                    formats[n].guid[1]);
                            fprintf(stdout, "%02x", (unsigned char)
                                    formats[n].guid[2]);
                            fprintf(stdout, "%02x-", (unsigned char)
                                    formats[n].guid[3]);
                            fprintf(stdout, "%02x", (unsigned char)
                                    formats[n].guid[4]);
                            fprintf(stdout, "%02x-", (unsigned char)
                                    formats[n].guid[5]);
                            fprintf(stdout, "%02x", (unsigned char)
                                    formats[n].guid[6]);
                            fprintf(stdout, "%02x-", (unsigned char)
                                    formats[n].guid[7]);
                            fprintf(stdout, "%02x", (unsigned char)
                                    formats[n].guid[8]);
                            fprintf(stdout, "%02x-", (unsigned char)
                                    formats[n].guid[9]);
                            fprintf(stdout, "%02x", (unsigned char)
                                    formats[n].guid[10]);
                            fprintf(stdout, "%02x", (unsigned char)
                                    formats[n].guid[11]);
                            fprintf(stdout, "%02x", (unsigned char)
                                    formats[n].guid[12]);
                            fprintf(stdout, "%02x", (unsigned char)
                                    formats[n].guid[13]);
                            fprintf(stdout, "%02x", (unsigned char)
                                    formats[n].guid[14]);
                            fprintf(stdout, "%02x\n", (unsigned char)
                                    formats[n].guid[15]);

                            fprintf(stdout, "        bits per pixel: %i\n",
                                    formats[n].bits_per_pixel);
                            fprintf(stdout, "        number of planes: %i\n",
                                    formats[n].num_planes);
                            fprintf(stdout, "        type: %s (%s)\n",
                                    (formats[n].type == XvRGB) ? "RGB" : "YUV",
                                    (formats[n].format ==
                                     XvPacked) ? "packed" : "planar");

                            if (formats[n].type == XvRGB) {
                                fprintf(stdout, "        depth: %i\n",
                                        formats[n].depth);

                                fprintf(stdout,
                                        "        red, green, blue masks: "
                                        "0x%x, 0x%x, 0x%x\n",
                                        formats[n].red_mask,
                                        formats[n].green_mask,
                                        formats[n].blue_mask);
                            }
                            else {

                            }
                        }

                    }
                    if (formats)
                        XFree(formats);
                }

                XvFreeEncodingInfo(encodings);
            }

        }

        XvFreeAdaptorInfo(ainfo);
    }
    return 0;
}
