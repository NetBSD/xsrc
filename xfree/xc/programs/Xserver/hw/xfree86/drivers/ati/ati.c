/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/ati.c,v 1.17 2001/05/09 03:12:02 tsi Exp $ */
/*
 * Copyright 1997 through 2001 by Marc Aurele La France (TSI @ UQV), tsi@xfree86.org
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
 * Author:  Marc Aurele La France (TSI @ UQV), tsi@xfree86.org
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
 * Rik Faith, faith@precisioninsight.com
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
 * Kevin Martin, martin@precisioninsight.com
 * Frederic Rienthaler, root@mojo.synapse.com
 * Marc Bolduc, bolduc@cim.mcgill.ca
 * Reuben Sumner, rasumner@undergrad.math.uwaterloo.ca
 * Benjamin T. Yang, risk@uclink.berkeley.edu
 * James Fast Kane, jfk2@engr.uark.edu
 * Randall Hopper, rhh@ct.picker.com
 * W. Marcus Miller, marcus@llnl.gov
 * Henrik Harmsen, ervhch@erv.ericsson.se
 * Christian Lupien, lupien@physics.utoronto.ca
 * Precision Insight Incorporated
 * Mark Vojkovich, mvojkovich@nvidia.com
 * Huw D M Davies, h.davies1@physics.ox.ac.uk
 * Andrew C Aitchison, A.C.Aitchison@dpmms.cam.ac.uk
 * Ani Joshi, ajoshi@shell.unixbox.com
 * Kostas Gewrgiou, gewrgiou@imbc.gr
 * Jakub Jelinek, jakub@redhat.com
 * David S. Miller, davem@redhat.com
 * A E Lawrence, adrian.lawrence@computing-services.oxford.ac.uk
 * Linus Torvalds, torvalds@transmeta.com
 * William Blew, wblew@home.com
 * Ignacio Garcia Etxebarria, garetxe@euskalnet.net
 *
 * ... and, many, many others from around the world.
 *
 * In addition, this work would not have been possible without the active
 * support, both moral and otherwise, of the staff and management of Computing
 * and Network Services at the University of Alberta, in Edmonton, Alberta,
 * Canada.
 *
 * The driver is intended to support all ATI adapters since their VGA Wonder
 * V3, including OEM counterparts.
 */

#include "atiident.h"
#include "atioption.h"
#include "atiprobe.h"
#include "ativersion.h"

/* The root of all evil... */
DriverRec ATI =
{
    ATI_VERSION_CURRENT,
    "ati",
    ATIIdentify,
    ATIProbe,
    ATIAvailableOptions,
    NULL,
    0
};
