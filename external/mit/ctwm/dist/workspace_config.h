/*
 * Routines using in setting up the config for workspace stuff.
 */
#ifndef _CTWM_WORKSPACE_CONFIG_H
#define _CTWM_WORKSPACE_CONFIG_H

void AddWorkSpace(const char *name,
                  const char *background, const char *foreground,
                  const char *backback,   const char *backfore,
                  const char *backpix);
void WMapCreateCurrentBackGround(const char *border,
                                 const char *background, const char *foreground,
                                 const char *pixmap);
void WMapCreateDefaultBackGround(const char *border,
                                 const char *background, const char *foreground,
                                 const char *pixmap);

#endif /* _CTWM_WORKSPACE_CONFIG_H */
