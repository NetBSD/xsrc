
contextsdir = $(rstartdir)/contexts
contexts_DATA =		\
	x11r6

dist_contexts_DATA = 	\
	default		\
	@List

dist_noinst_DATA =	\
	@Aliases	\
	x11r6.in

PATH_SUBSTS = \
	-e 's|@DEFAULT_X_MANPATH[@]|$(DEFAULT_X_MANPATH)|g' \
	-e 's|@DEFAULT_USER_PATH[@]|$(DEFAULT_USER_PATH)|g'

x11r6: x11r6.in
	$(AM_V_GEN)$(SED) $(PATH_SUBSTS) < $(srcdir)/$@.in > $@

CLEANFILES = $(contexts_DATA)

install-data-hook:
	$(AM_V_GEN)(cd $(DESTDIR)$(contextsdir) && rm -f x && $(LN_S) x11r6 x)
	$(AM_V_GEN)(cd $(DESTDIR)$(contextsdir) && rm -f x11 && $(LN_S) x11r6 x11)

uninstall-hook:
	$(AM_V_at)(cd $(DESTDIR)$(contextsdir) && rm -f x x11)
