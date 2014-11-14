#include "debug.h"
#ifndef WIN32
#include <unistd.h>
#endif
#include <stdlib.h>
#include <cstdio>
#include <signal.h>
#include <stdarg.h>

#define BREAK_SCRIPT_NAME "_____I_HATE_THE_WORLD_____"
#define BREAK_SIGNAL SIGUSR1

#ifndef WIN32
//#define USE_XTERM   // This works
//#define USE_DDD     // This is currently broken
#define USE_XEMACS_GDB
#else
#define USE_NT
#endif

void BreakHere() {
    fprintf(stderr, "Dummy Debug Function Execed\n");
    return;
}

char * GetExecName() {
  // DUX 4
#if defined(__osf1__)
    extern char **__Argv;
    return __Argv[0];
#else
    return 0;
#endif
}

void AttacheDebuggerHereSigHandler(int sig) {
    sig = 0; // avoid compiler warning
}


#if defined(USE_XTERM) || defined(USE_DDD)
void AttachDebuggerHere(char * execname = 0) {
    char s[1000];
    FILE * fd = NULL;

    if (execname == 0) {
	execname = GetExecName();
    }

    fd = fopen(BREAK_SCRIPT_NAME, "w");

#ifdef USE_XTERM
    fprintf(fd, "#!/bin/sh\necho \"break BreakHere\ncont\ncont\" > %s.glarp ; (cat %s.glarp - | gdb %s %d) \n",
	    BREAK_SCRIPT_NAME, BREAK_SCRIPT_NAME, execname, getpid());
#endif

#ifdef USE_DDD
    fprintf(fd,"#!/bin/sh\necho \"break BreakHere\ncont\ncont\" > %s.glarp ; (cat %s.glarp | ddd --gdb --tty %s %d) \n",
	    BREAK_SCRIPT_NAME, BREAK_SCRIPT_NAME, execname, getpid());
#endif

    fclose(fd);
    sprintf(s, "chmod +x %s", BREAK_SCRIPT_NAME);
    system(s);

    signal(SIGUSR2, &AttacheDebuggerHereSigHandler);

#ifdef USE_XTERM
    sprintf(s, "((xterm -fg cyan -bg black -sl 5000 -T \"Debug %s\" -e  %s) &) ; ((sleep 5; kill -s USR2 %d) &)", 
	    execname, BREAK_SCRIPT_NAME, getpid());
#endif

#ifdef USE_DDD
    sprintf(s, "( %s &) ; ((sleep 10; kill -s USR2 %d) &)", BREAK_SCRIPT_NAME,getpid());
#endif

    system(s);
    pause();
    signal(SIGUSR2, SIG_DFL);
    sprintf(s, "%s.glarp", BREAK_SCRIPT_NAME);
    unlink(BREAK_SCRIPT_NAME);
    unlink(s);
}
#endif

#if defined(USE_XEMACS_GDB)
void AttachDebuggerHere(char * execname) {
    char s[1000];

    if (execname == 0) {
	execname = GetExecName();
    }

    //signal(SIGUSR2,&AttacheDebuggerHereSigHandler);
    sprintf(s,  "( EXEC=`which %s`; gnudoit \"(progn (gdb \\\"$EXEC\\\") (gdb-call \\\"attach %d\\\") (gdb-call \\\"break BreakHere\\\") (gdb-call \\\"continue\\\") )\") ; sleep 10", 
	    execname, getpid());
 
    fprintf(stderr, "%s\n", s);
    system(s);
    //pause();
    //signal(SIGUSR2,SIG_DFL);
    //sprintf(s,"%s.glarp",BREAK_SCRIPT_NAME);
}
#endif

#if defined(USE_NT)
void AttachDebuggerHere(char *execname)
{
    fprintf(stderr,"AttachDebuggerHere not implemented on NT\n");
}

#endif


static FILE * db_file = DEFAULT_DEBUG_PRINT_FILE;

int MINET_DEBUGLEVEL = ((getenv("MINET_DEBUGLEVEL") == 0) ? DEFAULT_DEBUG_PRINT_LEVEL : atoi(getenv("MINET_DEBUGLEVEL")));


void DEBUGSETLEVEL(int level) {
    MINET_DEBUGLEVEL = level;
}

void DEBUGSETFILE(FILE *file) {
    db_file = file;
}

void DEBUGPRINTF(int level, const char * fmt, ...) {
    va_list list;

    va_start(list, fmt);

    if (level < MINET_DEBUGLEVEL) {
	vfprintf(db_file, fmt, list);
    }
}

DebugStream debug(std::cout);

