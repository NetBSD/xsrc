/*
 * Copyright 2014 SHS SERVICES GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _VIA_VT1632_H_
#define _VIA_VT1632_H_ 1

#define VIA_VT1632_VEN  0x20
#define VIA_VT1632_HEN  0x10
#define VIA_VT1632_DSEL 0x08
#define VIA_VT1632_BSEL 0x04
#define VIA_VT1632_EDGE 0x02
#define VIA_VT1632_PDB  0x01

struct ViaVT1632PrivateData {
	I2CDevPtr VT1632I2CDev;

	int DotclockMin;
	int DotclockMax;
	CARD8 Register08;
	CARD8 Register09;
	CARD8 Register0A;
	CARD8 Register0C;
};

void via_vt1632_power(xf86OutputPtr output, BOOL on);
void via_vt1632_save(xf86OutputPtr output);
void via_vt1632_restore(xf86OutputPtr output);
int via_vt1632_mode_valid(xf86OutputPtr output, DisplayModePtr pMode);
void via_vt1632_mode_set(xf86OutputPtr output, DisplayModePtr mode, DisplayModePtr adjusted_mode);
xf86OutputStatus via_vt1632_detect(xf86OutputPtr output);
BOOL via_vt1632_probe(ScrnInfoPtr pScrn, I2CDevPtr pDev);
struct ViaVT1632PrivateData * via_vt1632_init(ScrnInfoPtr pScrn, I2CDevPtr pDev);

#endif /* _VIA_VT1632_H_ */
