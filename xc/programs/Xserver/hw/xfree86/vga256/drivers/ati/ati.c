/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/ati.c,v 1.1.2.2 1999/10/12 17:18:51 hohndel Exp $ */
/*
 * Copyright 1997 through 1999 by Marc Aurele La France (TSI @ UQV), tsi@ualberta.ca
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of Marc Aurele La France not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Marc Aurele La France makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as-is" without express or implied warranty.
 *
 * MARC AURELE LA FRANCE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL MARC AURELE LA FRANCE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*************************************************************************/

/*
 * Author:  Marc Aurele La France (TSI @ UQV), tsi@ualberta.ca
 *
 * This is the ATI driver for XFree86.
 *
 * John Donne once said "No man is an island", and I am most certainly not an
 * exception.  Contributions, intentional or not, to this and previous versions
 * of this driver by the following are hereby acknowledged:
 *
 * Thomas Roell, roell@informatik.tu-muenchen.de
 * Per Lindqvist, pgd@compuram.bbt.se
 * Doug Evans, dje@cygnus.com
 * Rik Faith, faith@cs.unc.edu
 * Arthur Tateishi, ruhtra@turing.toronto.edu
 * Alain Hebert, aal@broue.rot.qc.ca
 * Ton van Rosmalen, ton@stack.urc.tue.nl
 * David Chambers, davidc@netcom.com
 * William Shubert, wms@ssd.intel.com
 * ATI Technologies Incorporated
 * Robert Wolff
 * David Dawes, dawes@xfree86.org
 * Mark Weaver, Mark_Weaver@brown.edu
 * Hans Nasten, nasten@everyware.se
 * Kevin Martin, martin@cs.unc.edu
 * Frederic Rienthaler, root@mojo.synapse.com
 * Marc Bolduc, bolduc@cim.mcgill.ca
 * Reuben Sumner, rasumner@undergrad.math.uwaterloo.ca
 * Benjamin T. Yang, risk@uclink.berkeley.edu
 * James Fast Kane, jfk2@engr.uark.edu
 * Randall Hopper, rhh@ct.picker.com
 * Christian Lupien <lupien@physics.utoronto.ca>
 *
 * ... and, many, many others from around the world.
 *
 * In addition, this work would not have been possible without the active
 * support, both moral and otherwise, of the staff and management of Computing
 * and Network Services at the University of Alberta, in Edmonton, Alberta,
 * Canada.
 *
 * The driver is intended to support the ATI VGA Wonder series of adapters and
 * its OEM counterpart, the VGA1024 series.  It will also work with Mach32's
 * and Mach64's but will not use their accelerated features.  This includes
 * Mach64's based on the 264xT series of integrated controllers.
 */

/*************************************************************************/

#include "ati.h"
#include "atiadjust.h"
#include "aticonsole.h"
#include "aticrtc.h"
#include "atifbinit.h"
#include "atigetmode.h"
#include "atiident.h"
#include "atiprobe.h"
#include "atireset.h"
#include "ativalid.h"

/*
 * This data structure defines the driver itself.  The data structure is
 * initialized with the functions that make up the driver and some data that
 * defines how the driver operates.  Some elements of this structure will be
 * modified by ATIProbe.
 */
vgaVideoChipRec ATI =
{
    ATIProbe,                   /* Probe */
    ATIIdent,                   /* Ident */
    ATIEnterLeave,              /* EnterLeave */
    ATIInit,                    /* Init */
    ATIValidMode,               /* ValidMode */
    ATISave,                    /* Save */
    ATIRestore,                 /* Restore */
    ATIAdjust,                  /* Adjust */
    ATISaveScreen,              /* SaveScreen */
    ATIGetMode,                 /* GetMode */
    ATIFbInit,                  /* FbInit */
    ATISetRead,                 /* SetRead */
    ATISetWrite,                /* SetWrite */
    ATISetReadWrite,            /* SetReadWrite */
    0x10000U,                   /* Mapped memory window size (64k) */
    0x10000U,                   /* Video memory bank size (64k) */
    16,                         /* Shift factor to get bank number */
    0xFFFFU,                    /* Bit mask for address within a bank */
    0x00000U, 0x10000U,         /* Boundaries for reads within a bank */
    0x00000U, 0x10000U,         /* Boundaries for writes within a bank */
    TRUE,                       /* Read & write banks can be different */
    -1,                         /* Not used in this driver */
    {{0,}},                     /* Options are set by ATIProbe */
    16,                         /* Virtual X rounding */
    FALSE,                      /* No linear frame buffer */
    0,                          /* Linear frame buffer base address */
    0,                          /* Linear frame buffer size */
    FALSE,                      /* No support for 16 bits per pixel (yet) */
    FALSE,                      /* No support for 24 bits per pixel (yet) */
    FALSE,                      /* No support for 32 bits per pixel (yet) */
    NULL,                       /* List of builtin modes */
    1,                          /* ChipClockMulFactor */
    1                           /* ChipClockDivFactor */
};
