
commandsdir = $(rstartdir)/commands
nobase_dist_commands_DATA =	\
	ListContexts		\
	x11r6/Terminal		\
	x11r6/LoadMonitor	\
	x11r6/@List		\
	@List			\
	ListGenericCommands

install-data-hook:
	$(AM_V_GEN)(cd $(DESTDIR)$(commandsdir) && rm -f x && $(LN_S) x11r6 x)
	$(AM_V_GEN)(cd $(DESTDIR)$(commandsdir) && rm -f x11 && $(LN_S) x11r6 x11)

uninstall-hook:
	$(AM_V_at)(cd $(DESTDIR)$(commandsdir) && rm -f x x11)
