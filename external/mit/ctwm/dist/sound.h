/*
 * Copyright 1992 Claude Lecommandeur.
 */

#ifndef _CTWM_SOUND_H
#define _CTWM_SOUND_H

void sound_init(void);
void sound_clear_list(void);
void sound_load_list(void);
void play_sound(int);
void play_startup_sound(void);
void play_exit_sound(void);
void sound_set_from_config(void);
void toggle_sound(void);
void reread_sounds(void);
void set_sound_host(char *);
int set_sound_event_name(const char *, const char *);
int set_sound_event(int, const char *);

#endif /* _CTWM_SOUND_H */
