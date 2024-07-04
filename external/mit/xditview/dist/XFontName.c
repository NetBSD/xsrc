/*
 * XFontName.c
 *
 * build/parse X Font name strings
 */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include	<X11/Xlib.h>
#include	<X11/Intrinsic.h>
#include	"XFontName.h"
#include	<ctype.h>

static const char *
extractStringField(const char *name, char *buffer, int size,
                   unsigned int *attrp, unsigned int bit)
{
    char *buf = buffer;

    if (!*name)
        return NULL;
    while (*name && *name != '-' && size > 0) {
        *buf++ = *name++;
        --size;
    }
    if (size <= 0)
        return NULL;
    *buf = '\0';
    if (buffer[0] != '*' || buffer[1] != '\0')
        *attrp |= bit;
    if (*name == '-')
        return name + 1;
    return name;
}

static const char *
extractUnsignedField(const char *name, unsigned int *result,
                     unsigned int *attrp, unsigned int bit)
{
    char buf[256];
    unsigned int i;

    name = extractStringField(name, buf, sizeof(buf), attrp, bit);
    if (!name)
        return NULL;
    if (!(*attrp & bit))
        return name;
    i = 0;
    for (char *c = buf; *c; c++) {
        if (!isdigit(*c))
            return NULL;
        i = i * 10 + (*c - '0');
    }
    *result = i;
    return name;
}

Bool
XParseFontName(const char *fontNameString, XFontName *fontName,
               unsigned int *fontNameAttributes)
{
    const char *name = fontNameString;
    XFontName temp;
    unsigned int attributes = 0;

#define GetString(field,bit)  do { \
	if (!(name = extractStringField \
		(name, temp.field, sizeof (temp.field),\
		&attributes, bit))) \
		return False; \
    } while(0)

#define GetUnsigned(field,bit)  do { \
	if (!(name = extractUnsignedField \
		(name, &temp.field, \
		&attributes, bit))) \
		return False; \
    } while(0)

    GetString(Registry, FontNameRegistry);
    GetString(Foundry, FontNameFoundry);
    GetString(FamilyName, FontNameFamilyName);
    GetString(WeightName, FontNameWeightName);
    GetString(Slant, FontNameSlant);
    GetString(SetwidthName, FontNameSetwidthName);
    GetString(AddStyleName, FontNameAddStyleName);
    GetUnsigned(PixelSize, FontNamePixelSize);
    GetUnsigned(PointSize, FontNamePointSize);
    GetUnsigned(ResolutionX, FontNameResolutionX);
    GetUnsigned(ResolutionY, FontNameResolutionY);
    GetString(Spacing, FontNameSpacing);
    GetUnsigned(AverageWidth, FontNameAverageWidth);
    GetString(CharSetRegistry, FontNameCharSetRegistry);
    if (!*name) {
        temp.CharSetEncoding[0] = '\0';
        attributes |= FontNameCharSetEncoding;
    }
    else {
        GetString(CharSetEncoding, FontNameCharSetEncoding);
    }
    *fontName = temp;
    *fontNameAttributes = attributes;
    return True;
}

static char *
utoa(unsigned int u, char *s, int size)
{
    char *t;

    t = s + size;
    *--t = '\0';
    do
        *--t = (u % 10) + '0';
    while (u /= 10);
    return t;
}

Bool
XFormatFontName(XFontName *fontName, unsigned int fontNameAttributes,
                XFontNameString fontNameString)
{
    XFontNameString tmp;
    char *name = tmp;
    const char *f;
    int left = sizeof(tmp) - 1;
    char number[32];

#define PutString(field, bit) do { \
	f = (fontNameAttributes & bit) ? \
		fontName->field \
		: "*"; \
	if ((left -= strlen (f)) < 0) \
		return False; \
	while (*f) \
		if ((*name++ = *f++) == '-') \
			return False; \
    } while(0)

#define PutHyphen() do { \
	if (--left < 0) \
		return False; \
	*name++ = '-'; \
    } while(0)

#define PutUnsigned(field, bit) do { \
	f = (fontNameAttributes & bit) ? \
		utoa (fontName->field, number, sizeof (number)) \
		: "*"; \
	if ((left -= strlen (f)) < 0) \
		return False; \
	while (*f) \
		*name++ = *f++; \
    } while(0)

    PutString(Registry, FontNameRegistry);
    PutHyphen();
    PutString(Foundry, FontNameFoundry);
    PutHyphen();
    PutString(FamilyName, FontNameFamilyName);
    PutHyphen();
    PutString(WeightName, FontNameWeightName);
    PutHyphen();
    PutString(Slant, FontNameSlant);
    PutHyphen();
    PutString(SetwidthName, FontNameSetwidthName);
    PutHyphen();
    PutString(AddStyleName, FontNameAddStyleName);
    PutHyphen();
    PutUnsigned(PixelSize, FontNamePixelSize);
    PutHyphen();
    PutUnsigned(PointSize, FontNamePointSize);
    PutHyphen();
    PutUnsigned(ResolutionX, FontNameResolutionX);
    PutHyphen();
    PutUnsigned(ResolutionY, FontNameResolutionY);
    PutHyphen();
    PutString(Spacing, FontNameSpacing);
    PutHyphen();
    PutUnsigned(AverageWidth, FontNameAverageWidth);
    PutHyphen();
    PutString(CharSetRegistry, FontNameCharSetRegistry);
    PutHyphen();
    PutString(CharSetEncoding, FontNameCharSetEncoding);
    *name = '\0';
    strncpy (fontNameString, tmp, sizeof(XFontNameString));
    fontNameString[sizeof(XFontNameString) - 1] = '\0';
    return True;
}
