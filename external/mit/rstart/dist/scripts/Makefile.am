
# config data

rstart_DATA = config
dist_noinst_DATA = config.in

CONFIG_SUBSTS = -e 's|@rstartdir[@]|$(rstartdir)|g'
config: config.in
	$(AM_V_GEN)$(SED) $(CONFIG_SUBSTS) < $(srcdir)/$@.in > $@

CLEANFILES = $(rstart_DATA)

# wrapper scripts

bin_SCRIPTS = rstart rstartd
dist_noinst_SCRIPTS = rstart.in rstartd.in

RSTART_SUBSTS = -e 's|@RSH[@]|$(RSH)|g'
rstart: rstart.in
	$(AM_V_GEN)$(SED) $(RSTART_SUBSTS) < $(srcdir)/$@.in > $@

RSTARTD_SUBSTS = -e 's|@rstartdir[@]|$(rstartdir)|g'
rstartd: rstartd.in
	$(AM_V_GEN)$(SED) $(RSTARTD_SUBSTS) < $(srcdir)/$@.in > $@

CLEANFILES += $(bin_SCRIPTS)
