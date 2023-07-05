/*
 * Signal handlers
 */

#include "ctwm.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ctwm_shutdown.h"
#include "signals.h"


/* Our backends */
static void sh_restart(int signum);
static void sh_shutdown(int signum);


// Internal flags for which signals have called us
static bool sig_restart = false;
static bool sig_shutdown = false;

// External flag for whether some signal handler has set a flag that
// needs to trigger an action.
bool SignalFlag = false;


/**
 * Setup signal handlers (run during startup)
 */
void
setup_signal_handlers(void)
{
	// INT/QUIT/TERM: shutdown
	// XXX Wildly unsafe handler; to be reworked
	signal(SIGINT,  sh_shutdown);
	signal(SIGQUIT, sh_shutdown);
	signal(SIGTERM, sh_shutdown);

	// SIGHUP: restart
	signal(SIGHUP, sh_restart);

	// We don't use alarm(), but if we get the stray signal we shouldn't
	// die...
	signal(SIGALRM, SIG_IGN);

	// This should be set by default, but just in case; explicitly don't
	// leave zombies.
	signal(SIGCHLD, SIG_IGN);

	return;
}


/**
 * Handle stuff set by a signal flag.  Could be a Restart, could be a
 * Shutdown...
 */
void
handle_signal_flag(Time t)
{
	// Restarting?
	if(sig_restart) {
		// In case it fails, don't loop
		sig_restart = false;

		// Handle
		DoRestart(t);

		// Shouldn't return, but exec() might fail...
		return;
	}

	// Shutting down?
	if(sig_shutdown) {
		// Doit
		DoShutdown();

		// Can't return!
		fprintf(stderr, "%s: DoShutdown() shouldn't return!\n", ProgramName);
		exit(1);
	}

	// ???
	fprintf(stderr, "%s: Internal error: unexpected signal flag.\n",
	        ProgramName);
	return;
}



/*
 * Internal backend bits
 */

/**
 * Set flag to restart.  Backend for SIGHUP.
 */
static void
sh_restart(int signum)
{
	// Signal handler; stdio isn't async-signal-safe, write(2) is
	const char srf[] = ":  signal received, setting restart flag\n";
	write(2, ProgramName, ProgramNameLen);
	write(2, srf, sizeof(srf));

	SignalFlag = sig_restart = true;
}

/**
 * Set flag to shutdown.  Backend for SIGTERM etc.
 */
static void
sh_shutdown(int signum)
{
	// Signal handler; stdio isn't async-signal-safe, write(2) is
	const char srf[] = ":  signal received, setting shutdown flag\n";
	write(2, ProgramName, ProgramNameLen);
	write(2, srf, sizeof(srf));

	SignalFlag = sig_shutdown = true;
}

