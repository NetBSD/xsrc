/*
 * Copyright 1990 Open Software Foundation (OSF)
 * Copyright 1990 Unix International (UI)
 * Copyright 1990 X/Open Company Limited (X/Open)
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of OSF, UI or X/Open not be used in 
 * advertising or publicity pertaining to distribution of the software 
 * without specific, written prior permission.  OSF, UI and X/Open make 
 * no representations about the suitability of this software for any purpose.  
 * It is provided "as is" without express or implied warranty.
 *
 * OSF, UI and X/Open DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO 
 * EVENT SHALL OSF, UI or X/Open BE LIABLE FOR ANY SPECIAL, INDIRECT OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF 
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR 
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR 
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

/************************************************************************

SCCS:          @(#)tcc_prot.h    1.9 03/09/92
NAME:          tcc_prot.h
PRODUCT:       TET (Test Environment Toolkit)
AUTHOR:        J B Goode, OSF
DATE CREATED:  14 May 1991
CONTENTS:      Prototypes and extern variables for TCC

MODIFICATIONS:

               "TET Rework"
               David G. Sawyer, UniSoft Ltd,  July 1991.

	       Changed declaration for obtain_tool_lock() to include char *
               David G. Sawyer, UniSoft Ltd,  31 Oct 1991.

	       Changed tc_search to resume_status
               David G. Sawyer, UniSoft Ltd,  13 Feb 1992.

			*** TET 1.9 for MIT X Test Suite ***
			Fixed wrong prototype declaration for tet_[re|m]alloc()
			Stuart Boutell, UniSoft Ltd, 12 March 1992

************************************************************************/

/* The TET environment structure */
typedef struct env_t
{
    char *name, *value;
    bool perm;
} ENV_T;


/* The Results Code structure */
typedef struct rescode {
    int num;
    char *name;
    char action[RESCODE_AC_LEN];
} RESCODE;


/* Test Purpose Result Structure */
typedef struct tprs  
{
    int num;     /* The message sequence number; ++ed for each entry */
    int context; /* The process that generated the entry */
    int block;   /* The message block number; ++ed by tp request */
    int type;    /* The type of message in the file */
    int tpnum;   /* This num uniquely identifies the tp within a tc */
    char *data;  /* The line of info in the file */

    struct tprs *next,*prev;
} TPRS;

#ifdef DBGMEM
typedef struct memcheck
{
	long	ptr;
	int	vol;
	char	file[50];
	int	line;
} MEMCHECK;
#endif
/*
 *    external definitions
 */
#ifdef DBGMEM
extern    MEMCHECK  *memtable;
#endif
extern    ENV_T     *tet_env;
extern    ENV_T     *bld_env;
extern    ENV_T     *exe_env;
extern    ENV_T     *cln_env;
extern    FILE      *old_jnl_fp;
extern    RESCODE   res_tbl[];
extern    bool      build_mode;
extern    bool      clean_mode;
extern    bool      exec_mode;
extern    bool      jnl_open_flag;
extern    char      alt_exec_dir[];
extern    char      *bld_cfg_file_name;
extern    char      *exe_cfg_file_name;
extern    char      *cln_cfg_file_name;
extern    char      *g_tc_line;
extern    char      line[];
extern    char      *optarg;
extern    char      results_dir[];
extern    char      *result_pat;    
extern    char      *scen_file_name;
extern    char      *suite_name;
extern    char      test_suite_root[];
extern    char      *scenario_name;
extern    char      temp_dir[];
extern    char      tet_root[];
extern    char      results_file[];
extern    char      **yes_list;
extern    char      **no_list;
extern    char      error_mesg[];
extern    char      **save_files;
extern    int       resume_status;
extern    int       context;
extern    int       cpid;
extern    int       debug_suite;
extern    int       exec_ctr;
extern    int       g_max_rescode;
extern    int       g_timeout;
extern    int       jnl_fd;
extern    int       optind;
extern    int       lock_type;
extern    int       tmode;
extern    int       exec_all_flag;
extern    int       *result_list;
extern    int       result_mode[];
extern    int       g_ic_num;
extern    int       interrupted;
extern    int       produce_output;
extern    int       alt_exec_set;
extern    int       context_status;
extern    int       oc_set;
extern    int       eip_set;
extern    int       abort_requested;
extern    int       scenario_line;
extern    int       scenario_element;
extern    int       old_line;
extern    int       old_element;

extern    sigjmp_buf   jmpbuf;


#if __STDC__

/**********   Function Prototypes For ANSI C   **********/

/* tcc.c */
extern    void      tool_sig_hdlr(int);
extern    void      exec_sig_hdlr(int);
extern    void      siginthdlr(int);
extern    void      sighdlr(int);
extern    char *    basename(char *);
extern    void      show_synopsis();
extern    void      default_cfg_file(char **, char *, char *, char *);
extern    void      tet_shutdown();
extern    void      tet_cleanup();
extern    void      tet_free(void *, char *, int);
#ifdef DBGMEM
extern    void      memory_table(int);
extern    void      mem_setup();
extern    void *    tet_malloc(size_t, char *, int, int);
extern    void *    tet_realloc(void *, size_t, char *, int, int);
#else
extern    void *    tet_malloc(size_t, char *, int, int); /* Patched XXX */
extern    void *    tet_realloc(void *, size_t, char *, int, int); /* XXX */
#endif


/* again.c */
extern    int       do_again(int);
extern    void      rerun_flush(int, int, int, int, char *, int *);
extern    char *    findstr(char *, char *);
extern    void      make_ic_list(char *, int **);
extern    int       compare_ics(const void  *, const void *);


/* tool.c */
extern    int       tool_tc(char *, int );
extern    int       do_tool(char *);
extern    int       obtain_tool_lock(char *);
extern    int       obtain_exe_excl_lock();
extern    int       obtain_exe_shrd_lock();
extern    void      release_tool_lock();
extern    void      release_exe_excl_lock();
extern    void      release_exe_shrd_lock();


/* config.c */
extern    void      init_config( int); 
extern    void      do_config_file(char *, int);
extern    void      add_tetenv(char *, char *, int);
extern    char *    get_tetenv(char *);
extern    char *    rescode_num_to_name(int);
extern    int       tet_putenv(char *);
extern    void      dump_config(char *, int);


/* exec_mode.c */
extern    int       do_mkdir(char *);
extern    int       do_copy(char *);
extern    void      do_save();
extern    void      do_rm(char *);


/* log_entry.c */
extern    char      *get_jnl_dir();
extern    void      get_time();
extern    void      jnl_tool_end(int, int, char *, int);
extern    void      jnl_tool_start(int, char *, char *, int);
extern    void      jnl_config_end();
extern    void      jnl_config_start(char *, int);
extern    void      jnl_config_value(char *);
extern    void      jnl_entry(char *);
extern    void      jnl_entry_captured(int, char *);
extern    void      jnl_entry_invoke_tc(int, char *, char *);
extern    void      jnl_entry_scen(char *);
extern    void      jnl_entry_tcc_end(char *);
extern    void      jnl_entry_tcc_start(char *);
extern    void      jnl_tc_end(int, int, char *);
extern    void      jnl_tc_message(char *);
extern    void      jnl_tp_result(int, int, int, char *);
extern    void      jnl_user_abort(char *);
extern    void      tet_error(char *, char *, int);


/* scenario.c */
extern    int       perform_scen();
extern    void      process_line(char *);
extern    int       exec_tc(char *);
extern    int       get_rescode_file();
extern    int       do_rescode_file(FILE *);
extern    char *    rescode_num_to_action(int);
extern    int       rescode_name_to_num(char *);
extern    int       rescode_num_to_index(int);
extern    int       check_line(char *);
extern    char *    number_suffix( int);


/* startit.c */
extern    int       start_tc(char *, char *);
extern    int       export(ENV_T *); 
extern    int       copy_results_file(FILE *);
extern    int       tpstart(char *);
extern    int       tpend(char *, int);
extern    int       icstart(char *);
extern    int       icend(char *);
extern    int       modestart(char *);
extern    int       modeend(char *);
extern    int       make_tprs_list(FILE *, char *);
extern    void      parse_tprs_list();
extern    void      pr_list_to_jnl(TPRS *);
extern    TPRS *    collate_and_sort(TPRS *, int);
extern    int       compare_seq_num(const void *, const void *);
extern    void      pr_array_to_jnl(TPRS **, int);
extern    int       analyze_jnl_entry(char *, int *, int *, int *, int *);

/**********   End of ANSI C Prototypes   ***********/

#else

/**********   Function Declarations For Non-ANSI C   **********/

/* tcc.c */
extern    void      tool_sig_hdlr();
extern    void      exec_sig_hdlr();
extern    void      siginthdlr();
extern    void      sighdlr();
extern    char *    basename();
extern    void      show_synopsis();
extern    void      default_cfg_file();
extern    void      tet_shutdown();
extern    void      tet_cleanup();
extern    void      tet_free();
extern    void *    tet_malloc();
extern    void *    tet_realloc();
#ifdef DBGMEM
extern    void      memory_table();
extern    void      mem_setup();
#endif


/* again.c */
extern    int       do_again();
extern    void      rerun_flush();
extern    char *    findstr();
extern    void      make_ic_list();
extern    int       compare_ics();


/* tool.c */
extern    int       tool_tc();
extern    int       do_tool();
extern    int       obtain_tool_lock();
extern    int       obtain_exe_excl_lock();
extern    int       obtain_exe_shrd_lock();
extern    void      release_tool_lock();
extern    void      release_exe_excl_lock();
extern    void      release_exe_shrd_lock();


/* config.c */
extern    void      init_config(); 
extern    void      do_config_file();
extern    void      add_tetenv();
extern    char *    get_tetenv();
extern    char *    rescode_num_to_name();
extern    int       tet_putenv();
extern    void      dump_config();


/* exec_mode.c */
extern    int       do_mkdir();
extern    int       do_copy();
extern    void      do_save();
extern    void      do_rm();


/* log_entry.c */
extern    char      *get_jnl_dir();
extern    void      get_time();
extern    void      jnl_tool_end();
extern    void      jnl_tool_start();
extern    void      jnl_config_end();
extern    void      jnl_config_start();
extern    void      jnl_config_value();
extern    void      jnl_entry();
extern    void      jnl_entry_captured();
extern    void      jnl_entry_invoke_tc();
extern    void      jnl_entry_scen();
extern    void      jnl_entry_tcc_end();
extern    void      jnl_entry_tcc_start();
extern    void      jnl_tc_end();
extern    void      jnl_tc_message();
extern    void      jnl_tp_result();
extern    void      jnl_user_abort();
extern    void      tet_error();


/* scenario.c */
extern    int       perform_scen();
extern    void      process_line();
extern    int       exec_tc();
extern    int       get_rescode_file();
extern    int       do_rescode_file();
extern    char *    rescode_num_to_action();
extern    int       rescode_name_to_num();
extern    int       rescode_num_to_index();
extern    int       check_line();
extern    char *    number_suffix();


/* startit.c */
extern    int       start_tc();
extern    int       export(); 
extern    int       copy_results_file();
extern    int       tpstart();
extern    int       tpend();
extern    int       icstart();
extern    int       icend();
extern    int       modestart();
extern    int       modeend();
extern    int       make_tprs_list();
extern    void      parse_tprs_list();
extern    void      pr_list_to_jnl();
extern    TPRS *    collate_and_sort();
extern    int       compare_seq_num();
extern    void      pr_array_to_jnl();
extern    int       analyze_jnl_entry();

#endif

/**********   End of Non-ANSI C Function Declarations   **********/
