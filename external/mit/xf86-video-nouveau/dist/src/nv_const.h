#ifndef __NV_CONST_H__
#define __NV_CONST_H__

#define NV_VERSION 4000
#define NV_NAME "NOUVEAU"
#define NV_DRIVER_NAME "nouveau"

typedef enum {
    OPTION_SW_CURSOR,
    OPTION_HW_CURSOR,
    OPTION_NOACCEL,
    OPTION_SHADOW_FB,
    OPTION_VIDEO_KEY,
    OPTION_WFB,
    OPTION_GLX_VBLANK,
    OPTION_ZAPHOD_HEADS,
    OPTION_PAGE_FLIP,
    OPTION_SWAP_LIMIT,
    OPTION_ASYNC_COPY,
    OPTION_ACCELMETHOD,
} NVOpts;


static const OptionInfoRec NVOptions[] = {
    { OPTION_SW_CURSOR,         "SWcursor",     OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_HW_CURSOR,         "HWcursor",     OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_NOACCEL,           "NoAccel",      OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_SHADOW_FB,         "ShadowFB",     OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_VIDEO_KEY,		"VideoKey",	OPTV_INTEGER,	{0}, FALSE },
    { OPTION_WFB,		"WrappedFB",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_GLX_VBLANK,	"GLXVBlank",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_ZAPHOD_HEADS,	"ZaphodHeads",	OPTV_STRING,	{0}, FALSE },
    { OPTION_PAGE_FLIP,		"PageFlip",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_SWAP_LIMIT,	"SwapLimit",	OPTV_INTEGER,	{0}, FALSE },
    { OPTION_ASYNC_COPY,	"AsyncUTSDFS",	OPTV_BOOLEAN,	{0}, FALSE },
    { OPTION_ACCELMETHOD,	"AccelMethod",	OPTV_STRING,	{0}, FALSE },
    { -1,                       NULL,           OPTV_NONE,      {0}, FALSE }
};

#endif /* __NV_CONST_H__ */
          
