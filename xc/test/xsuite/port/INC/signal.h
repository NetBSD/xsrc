/* $XConsortium: signal.h,v 1.1 92/06/11 15:29:43 rws Exp $ */
#ifndef	SIG_DFL

typedef	long	sigset_t;

struct sigaction {
	void		(*sa_handler)();
	sigset_t	sa_mask;
	int		sa_flags;
};

/*
 * For BSD use jmp_buf for sigjmp_buf.
 * For sig{set,long}jmp() alway assume that save is set (as it is in
 * the TCM)
 * (This would all be defined in setjmp.h on a POSIX system)
 */
#define sigjmp_buf	jmp_buf
#define sigsetjmp(env, save)	setjmp(env)
#define siglongjmp(env, val)	longjmp(env, val)

#define	SIG_DFL		(void (*)())0
#ifdef	lint
#define	SIG_IGN		(void (*)())(31)	/* ??? */
#else
#define	SIG_IGN		(void (*)())1
#endif
#define SIG_ERR		((void (*)())-1)

#define NSIG	32
#define	SIGHUP	1	/* hangup */
#define	SIGINT	2	/* interrupt */
#define	SIGQUIT	3	/* quit */
#define	SIGILL	4	/* illegal instruction (not reset when caught) */
#define	SIGTRAP	5	/* trace trap (not reset when caught) */
#define	SIGIOT	6	/* IOT instruction */
#define	SIGABRT	6	/* User Abort - aliased to IOT */
#define	SIGEMT	7	/* EMT instruction */
#define	SIGFPE	8	/* floating point exception (see machine/fpu.h) */
#define	SIGKILL	9	/* kill (cannot be caught or ignored) */
#define	SIGBUS	10	/* bus error */
#define	SIGSEGV	11	/* segmentation violation */
#define	SIGSYS	12	/* bad argument to system call */
#define	SIGPIPE	13	/* write on a pipe with no one to read it */
#define	SIGALRM	14	/* alarm clock */
#define	SIGTERM	15	/* software termination signal from kill */
#define	SIGURG	16	/* urgent condition on IO channel */
#define	SIGSTOP	17	/* sendable stop signal not from tty */
#define	SIGTSTP	18	/* stop signal from tty */
#define	SIGCONT	19	/* continue a stopped process */
#define	SIGCHLD	20	/* to parent on child stop or exit */
#define	SIGTTIN	21	/* to readers pgrp upon background tty read */
#define	SIGTTOU	22	/* like TTIN for output if (tp->t_local&LTOSTOP) */
#define	SIGIO	23	/* input/output possible signal */
#define	SIGXCPU	24	/* exceeded CPU time limit */
#define	SIGXFSZ	25	/* exceeded file size limit */
#define	SIGVTALRM 26	/* virtual time alarm */
#define	SIGPROF	27	/* profiling time alarm */
#define	SIGWINCH 28	/* window size change */
#define SIGUSR1 30	/* user defined signal 1 */
#define SIGUSR2 31	/* user defined signal 2 */

#define SA_NOCLDSTOP	2	/* same as SV_BSDSIG */

#define SIG_BLOCK	1
#define SIG_UNBLOCK	2
#define SIG_SETMASK	3

extern int	kill(), sigaction(), sigaddset(), sigdelset(),
		sigemptyset(), sigfillset(), sigismember(),
		sigpending(), sigprocmask(), sigsuspend();

extern	void	(*signal())();

#endif
