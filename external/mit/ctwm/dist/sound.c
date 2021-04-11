/*
 * These routines were extracted from the sound hack for olvwm3.3 by
 * Andrew "Ender" Scherpbier (turtle@sciences.sdsu.edu, Andrew@SDSU.Edu)
 * and modified by J.E. Sacco (jsacco @ssl.com) for tvtwm and twm.  They
 * were then slightly adapted for ctwm by Mark Boyns (boyns@sdsu.edu),
 * and have since been reworked more.
 */

#include "ctwm.h"

#include <rplay.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "event_names.h"
#include "sound.h"

RPLAY **rp = NULL;

static int need_sound_init = 1;
static int sound_from_config = 0;
static int sound_fd = 0;
static int sound_state = 1;
#define HOSTNAME_LEN 200
static char hostname[HOSTNAME_LEN];

/*
 * Function to trim away spaces at the start and end of a string
 */
static char *
trim_spaces(char *str)
{
	if(str != NULL) {
		char *p = str + strlen(str);
		while(*str != '\0' && *str != '\r' && *str != '\n' && isspace(*str)) {
			str++;
		}
		/* Assume all line end characters are at the end */
		while(p > str && isspace(p[-1])) {
			p--;
		}
		*p = '\0';
	}
	return str;
}

/*
 * Define stuff related to "magic" names.
 */
static const char *magic_events[] = {
	"Startup",
	"Shutdown",
};
#define NMAGICEVENTS (sizeof(magic_events) / sizeof(*magic_events))

static int
sound_magic_event_name2num(const char *name)
{
	int i;

	for(i = 0 ; i < NMAGICEVENTS ; i++) {
		if(strcasecmp(name, magic_events[i]) == 0) {
			/* We number these off the far end of the non-magic events */
			return event_names_size() + i;
		}
	}

	return -1;
}


/*
 * Now we know how many events we need to store up info for
 */
#define NEVENTS (event_names_size() + NMAGICEVENTS)



/*
 * Initialize the subsystem and its necessary bits
 */
void
sound_init(void)
{
	if(!need_sound_init) {
		return;
	}

	/* Can't happen */
	if(sound_fd != 0) {
		fprintf(stderr, "BUG: sound_fd not set but sound inited.\n");
		exit(1);
	}

	need_sound_init = 0;
	if(hostname[0] == '\0') {
		strncpy(hostname, rplay_default_host(), HOSTNAME_LEN - 1);
		hostname[HOSTNAME_LEN - 1] = '\0'; /* JIC */
	}

	if((sound_fd = rplay_open(hostname)) < 0) {
		rplay_perror("create");
	}

	/*
	 * Init rp if necessary
	 */
	if(rp == NULL) {
		if((rp = calloc(NEVENTS, sizeof(RPLAY *))) == NULL) {
			perror("calloc() rplay control");
			exit(1);
			/*
			 * Should maybe just bomb out of sound stuff, but there's
			 * currently no provision for that.  If malloc fails, we're
			 * pretty screwed anyway, so it's not much loss to just die.
			 */
		}
	}
}


/*
 * Clear out any set sounds
 */
void
sound_clear_list(void)
{
	int i;

	/* JIC */
	if(rp == NULL) {
		return;
	}

	/*
	 * Destroy any old sounds
	 */
	for(i = 0; i < NEVENTS; i++) {
		if(rp[i] != NULL) {
			rplay_destroy(rp[i]);
		}
		rp[i] = NULL;
	}
}


/*
 * [Re]load the sounds
 */
void
sound_load_list(void)
{
	FILE *fl;
	char *home;
	char *soundfile;
	char buffer[100];

	/* Guard; shouldn't be possible */
	if(rp == NULL) {
		fprintf(stderr, "Tried to load sounds before subsystem inited.\n");
		exit(1);
	}

	/* Find the .ctwm-sounds file */
	if((home = getenv("HOME")) == NULL) {
		home = "";
	}
	if(asprintf(&soundfile, "%s/.ctwm-sounds", home) < 0) {
		perror("Failed building path to sound file");
		return;
	}
	fl = fopen(soundfile, "r");
	free(soundfile);

	/*
	 * If it was found, but we already have sound set from the config
	 * file, complain on stderr and then return.
	 */
	if(fl != NULL && sound_from_config) {
		fprintf(stderr, "RplaySounds set in ctwmrc, not reading "
		        "~/.ctwm-sounds.\n");
		fclose(fl);
		return;
	}

	/* Clear out the old list, whether we have new or not */
	sound_clear_list();

	/* If there wasn't a .ctwm-sounds file, we're done now */
	if(fl == NULL) {
		return;
	}

	/* Now go ahead and parse it in */
	while(fgets(buffer, 100, fl) != NULL) {
		char *ename, *sndfile;

		ename = trim_spaces(strtok(buffer, ": \t"));
		if(ename == NULL || *ename == '#') {
			continue;
		}

		sndfile = trim_spaces(strtok(NULL, "\r\n"));
		if(sndfile == NULL || *sndfile == '#') {
			continue;
		}

		if(set_sound_event_name(ename, sndfile) != 0) {
			fprintf(stderr, "Error adding sound for %s; maybe event "
			        "name is invalid?\n", ename);
		}
	}
	fclose(fl);
}


/*
 * Play sound
 */
void
play_sound(int snd)
{
	/* Bounds */
	if(snd < 0 || snd >= NEVENTS) {
		return;
	}

	/* Playing enabled */
	if(sound_state == 0) {
		return;
	}

	/* Better already be initted */
	if(need_sound_init) {
		fprintf(stderr, "BUG: play_sound() Sound should be initted already.\n");
		return;
	}

	/* Skip if this isn't a sound we have set */
	if(rp[snd] == NULL) {
		return;
	}

	/* And if all else fails, play it */
	if(rplay(sound_fd, rp[snd]) < 0) {
		rplay_perror("rplay");
	}
}

void
play_startup_sound(void)
{
	play_sound(sound_magic_event_name2num("Startup"));
}

void
play_exit_sound(void)
{
	play_sound(sound_magic_event_name2num("Shutdown"));
}


/*
 * Flag that we loaded sounds from the ctwmrc
 */
void
sound_set_from_config(void)
{
	sound_from_config = 1;
}


/*
 * Toggle the sound on/off
 */
void
toggle_sound(void)
{
	sound_state ^= 1;
}


/*
 * Re-read the sounds mapping file
 */
void
reread_sounds(void)
{
	sound_load_list();
}

/*
 * Set the SoundHost and force the sound_fd to be re-opened.
 */
void
set_sound_host(char *host)
{
	strncpy(hostname, host, HOSTNAME_LEN - 1);
	hostname[HOSTNAME_LEN - 1] = '\0'; /* JIC */
	if(sound_fd != 0) {
		rplay_close(sound_fd);
	}
	sound_fd = 0;
}

/*
 * Set the sound to play for a given event
 */
int
set_sound_event_name(const char *ename, const char *soundfile)
{
	int i;

	/* Find the index we'll use in rp[] for it */
	i = sound_magic_event_name2num(ename);
	if(i < 0) {
		i = event_num_by_name(ename);
	}
	if(i < 0) {
		return -1;
	}

	/* Gotcha */
	return set_sound_event(i, soundfile);
}

int
set_sound_event(int snd, const char *soundfile)
{
	/* This shouldn't get called before things are initialized */
	if(rp == NULL) {
		fprintf(stderr, "%s(): internal error: called before initialized.\n", __func__);
		exit(1);
	}

	/* Cleanup old if necessary */
	if(rp[snd] != NULL) {
		rplay_destroy(rp[snd]);
	}

	/* Setup new */
	rp[snd] = rplay_create(RPLAY_PLAY);
	if(rp[snd] == NULL) {
		rplay_perror("create");
		return -1;
	}
	if(rplay_set(rp[snd], RPLAY_INSERT, 0, RPLAY_SOUND, soundfile, NULL)
	                < 0) {
		rplay_perror("rplay");
	}

	return 0;
}
