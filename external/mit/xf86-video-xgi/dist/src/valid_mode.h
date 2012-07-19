/* Copyright (C) 2003-2006 by XGI Technology, Taiwan.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL XGI AND/OR
 *  ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _ValidMode_h

#define _ValidMode_h

typedef struct _SupportMode
{
        int    HDisplay;
        int    VDisplay;
        int    Clock;
} SupportMode;

SupportMode *XgiMode;

SupportMode XG20_Mode[]= {
{640, 480, 25175}, 
{640, 480, 31500}, 
{640, 480, 36000}, 
{640, 480, 44900}, 
{640, 480, 56250},
{800, 600, 40000}, 
{800, 600, 50000}, 
{800, 600, 49500}, 
{800, 600, 56300}, 
{800, 600, 75800}, 
{800, 600, 84800}, 
{800, 600, 109175},
{1024, 768, 65000}, 
{1024, 768, 75000}, 
{1024, 768, 78750}, 
{1024, 768, 94500}, 
{1024, 768, 104998}, 
{1024, 768, 132258}, 
{1024, 768, 192069},
{1152, 864, 80350}, 
{1152, 864, 108000},
{1280, 960, 108000}, 
{1280, 960, 125999}, 
{1280, 960, 148500}, 
{1280, 960, 178992}, 
{1280, 960, 217325},
{1280, 1024, 108000}, 
{1280, 1024, 135000}, 
{1280, 1024, 157500}, 
{1280, 1024, 190960}, 
{1280, 1024, 233793},
/* {1440, 900,  106470}, *//* Jong@09292009; 1440x900@60 */
{1600, 1200, 162000}, 
{1600, 1200, 175500}, 
{1600, 1200, 189000}, 
{1600, 1200, 202500}, 
{1600, 1200, 229500},
/* {1680, 1050, 147140}, *//* Jong@09292009; 1680x1050@60 */
{0, 0, 0}
};


SupportMode XGI_Mode[]= {
{640, 400, 25175},
{640, 480, 25200}, {640, 480, 31500}, {640, 480, 36000}, {640, 480, 44900}, {640, 480, 56250}, {640, 480, 67765}, {640, 480, 86600},
{800, 600, 36000}, {800, 600, 40000}, {800, 600, 50000}, {800, 600, 49500}, {800, 600, 56300}, {800, 600, 75800}, {800, 600, 84800}, {800, 600, 109175},
{1024, 768, 44900}, {1024, 768, 65000}, {1024, 768, 75000}, {1024, 768, 78800}, {1024, 768, 94500}, {1024, 768, 104998}, {1024, 768, 132258}, {1024, 768, 192069},
{1152, 864, 80350}, {1152, 864, 108000},
{1280, 960, 108000}, {1280, 960, 120839}, {1280, 960, 125999}, {1280, 960, 148500}, {1280, 960, 178992},
{1280, 960, 217325}, {1280, 960, 299505},
{1280, 1024, 78750}, {1280, 1024, 108000}, {1280, 1024, 135000}, {1280, 1024, 157500}, {1280, 1024, 190960},
{1280, 1024, 233793}, {1280, 1024, 322273},
{1600, 1200, 162000}, {1600, 1200, 175500}, {1600, 1200, 189000}, {1600, 1200, 202500}, {1600, 1200, 229500},
{1600, 1200, 269655}, {1600, 1200, 323586},
{1920, 1440, 234000}, {1920, 1440, 252699}, {1920, 1440, 272041}, {1920, 1440, 297000}, {1920, 1440, 330615},
{1920, 1440, 388631},
{2048, 1536, 265728}, {2048, 1536, 286359}, {2048, 1536, 309789}, {2048, 1536, 332177}, {2048, 1536, 375847},
{800, 480, 39770}, {800, 480, 49500}, {800, 480, 56250},
{1024, 576, 65000}, {1024, 576, 78750}, {1024, 576, 94500},
{1280, 720, 108200}, {1280, 720, 135500}, {1280, 720, 157500},
{720, 480, 28322},
{720, 576, 36000},
{856, 480, 36000},
{1280, 768, 80000},
{1400, 1050, 126260},

{800, 450, 27740},
{0, 0, 0}
};

#endif
/* {108000, 1280, 960}, */
/* {80350, 1152, 864}, {108000, 1152, 864}, */
