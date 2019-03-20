#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <xorg-server.h>
#include <xorgVersion.h>
#include <xf86Parser.h>

#include "intel_options.h"

const OptionInfoRec intel_options[] = {
	{OPTION_ACCEL_ENABLE,	"Accel",	OPTV_BOOLEAN,	{0},	0},
	{OPTION_ACCEL_METHOD,	"AccelMethod",	OPTV_STRING,	{0},	0},
	{OPTION_BACKLIGHT,	"Backlight",	OPTV_STRING,	{0},	0},
	{OPTION_EDID,		"CustomEDID",	OPTV_STRING,	{0},	0},
	{OPTION_DRI,		"DRI",		OPTV_STRING,	{0},	0},
	{OPTION_PRESENT,	"Present",	OPTV_BOOLEAN,	{0},	1},
	{OPTION_COLOR_KEY,	"ColorKey",	OPTV_INTEGER,	{0},	0},
	{OPTION_VIDEO_KEY,	"VideoKey",	OPTV_INTEGER,	{0},	0},
	{OPTION_TILING_2D,	"Tiling",	OPTV_BOOLEAN,	{0},	1},
	{OPTION_TILING_FB,	"LinearFramebuffer",	OPTV_BOOLEAN,	{0},	0},
	{OPTION_ROTATION,	"HWRotation",	OPTV_BOOLEAN,	{0},	1},
	{OPTION_VSYNC,		"VSync",	OPTV_BOOLEAN,	{0},	1},
	{OPTION_PAGEFLIP,	"PageFlip",	OPTV_BOOLEAN,	{0},	1},
	{OPTION_SWAPBUFFERS_WAIT, "SwapbuffersWait", OPTV_BOOLEAN,	{0},	1},
	{OPTION_TRIPLE_BUFFER,	"TripleBuffer", OPTV_BOOLEAN,	{0},	1},
	{OPTION_PREFER_OVERLAY, "XvPreferOverlay", OPTV_BOOLEAN, {0}, 0},
	{OPTION_HOTPLUG,	"HotPlug",	OPTV_BOOLEAN,	{0},	1},
	{OPTION_REPROBE,	"ReprobeOutputs", OPTV_BOOLEAN,	{0},	0},
#ifdef INTEL_XVMC
	{OPTION_XVMC,		"XvMC",		OPTV_BOOLEAN,	{0},	1},
#endif
#ifdef USE_SNA
	{OPTION_ZAPHOD,		"ZaphodHeads",	OPTV_STRING,	{0},	0},
	{OPTION_VIRTUAL,	"VirtualHeads",	OPTV_INTEGER,	{0},	0},
	{OPTION_TEAR_FREE,	"TearFree",	OPTV_BOOLEAN,	{0},	0},
	{OPTION_CRTC_PIXMAPS,	"PerCrtcPixmaps", OPTV_BOOLEAN,	{0},	0},
#endif
#ifdef USE_UXA
	{OPTION_FALLBACKDEBUG,	"FallbackDebug",OPTV_BOOLEAN,	{0},	0},
	{OPTION_DEBUG_FLUSH_BATCHES, "DebugFlushBatches", OPTV_BOOLEAN, {0}, 0},
	{OPTION_DEBUG_FLUSH_CACHES, "DebugFlushCaches", OPTV_BOOLEAN, {0}, 0},
	{OPTION_DEBUG_WAIT, "DebugWait", OPTV_BOOLEAN, {0}, 0},
	{OPTION_BUFFER_CACHE,	"BufferCache",	OPTV_BOOLEAN,   {0},    1},
#endif
	{-1,			NULL,		OPTV_NONE,	{0},	0}
};

OptionInfoPtr intel_options_get(ScrnInfoPtr scrn)
{
	OptionInfoPtr options;

	xf86CollectOptions(scrn, NULL);
	if (!(options = malloc(sizeof(intel_options))))
		return NULL;

	memcpy(options, intel_options, sizeof(intel_options));
	xf86ProcessOptions(scrn->scrnIndex, scrn->options, options);

	return options;
}

Bool intel_option_cast_to_bool(OptionInfoPtr options, int id, Bool val)
{
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,7,99,901,0)
	xf86getBoolValue(&val, xf86GetOptValString(options, id));
#endif
	return val;
}

static int
namecmp(const char *s1, const char *s2)
{
	char c1, c2;

	if (!s1 || *s1 == 0) {
		if (!s2 || *s2 == 0)
			return 0;
		else
			return 1;
	}

	while (*s1 == '_' || *s1 == ' ' || *s1 == '\t')
		s1++;

	while (*s2 == '_' || *s2 == ' ' || *s2 == '\t')
		s2++;

	c1 = isupper(*s1) ? tolower(*s1) : *s1;
	c2 = isupper(*s2) ? tolower(*s2) : *s2;
	while (c1 == c2) {
		if (c1 == '\0')
			return 0;

		s1++;
		while (*s1 == '_' || *s1 == ' ' || *s1 == '\t')
			s1++;

		s2++;
		while (*s2 == '_' || *s2 == ' ' || *s2 == '\t')
			s2++;

		c1 = isupper(*s1) ? tolower(*s1) : *s1;
		c2 = isupper(*s2) ? tolower(*s2) : *s2;
	}

	return c1 - c2;
}

unsigned intel_option_cast_to_unsigned(OptionInfoPtr options, int id, unsigned val)
{
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,7,99,901,0)
	const char *str = xf86GetOptValString(options, id);
#else
	const char *str = NULL;
#endif
	unsigned v;

	if (str == NULL || *str == '\0')
		return val;

	if (namecmp(str, "on") == 0)
		return val;
	if (namecmp(str, "true") == 0)
		return val;
	if (namecmp(str, "yes") == 0)
		return val;

	if (namecmp(str, "0") == 0)
		return 0;
	if (namecmp(str, "off") == 0)
		return 0;
	if (namecmp(str, "false") == 0)
		return 0;
	if (namecmp(str, "no") == 0)
		return 0;

	v = atoi(str);
	if (v)
		return v;

	return val;
}
