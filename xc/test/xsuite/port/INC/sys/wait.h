/* $XConsortium: wait.h,v 1.1 92/06/11 15:30:32 rws Exp $ */
#define WNOHANG		1
#define WUNTRACED	2

#define WIFEXITED(s)	(((s) & 0xff) == 0)
#define WEXITSTATUS(s)	((s)>>8 & 0xff)

#define WIFSIGNALED(s)	(((s) & 0xff00) == 0 && WTERMSIG(s) != 0)
#define WTERMSIG(s)	((s) & 0x7f)

#define WIFSTOPPED(s)	(((s) & 0xff) == 0177)
#define WSTOPSIG(s)	((s)>>8 & 0xff)

extern pid_t wait(), waitpid();
