extern void KbdGetMapping(InputInfoPtr pInfo, KeySymsPtr pKeySyms,
				CARD8 *pModMap);

#ifdef __NetBSD__
#include <dev/wscons/wsconsio.h>
#endif
