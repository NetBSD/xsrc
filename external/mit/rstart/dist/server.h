
#ifndef _server_h
#define _server_h

extern void nomem(void);

/* auth.c */
extern void do_auth(void);
extern void key_auth(int ac, char **av);
extern void key_internal_auth_input(int ac, char **av);
extern void key_internal_auth_program(int ac, char **av);

extern char myname[];

#endif /* _server_h */
