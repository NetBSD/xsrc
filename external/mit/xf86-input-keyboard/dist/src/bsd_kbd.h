extern void KbdGetMapping(InputInfoPtr pInfo, KeySymsPtr pKeySyms,
				CARD8 *pModMap);
#ifdef USE_WSKBD_GETMAP
extern void KbdGetMappingFromWsksym(InputInfoPtr pInfo, KeySymsPtr pKeySyms,
				    CARD8 *pModMap);
#endif

