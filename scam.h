
/*
 *  Main Author : John C. Lusth
 *  Barely Authors : Jeffrey Robinson, Gabriel Loewen
 *  Last Edit : May 4, 2014
 *
 *  The layout of this document is such:
 *      #defines
 *          These are needed in two different places
 *
 *      extern global variables
 *          These are needed in many places
 *
 */

#ifndef __SCAM_H__
#define __SCAM_H__

#include <stdio.h>

/* Needed only in util.c and cell.c */
#define DEFAULT_GC 0
#define STOP_AND_COPY 0
#define MARK_SWEEP 1


/* Global variables */
extern char *PROGRAM_NAME;      /* The name of the program (scam) */
extern char *PROGRAM_VERSION;   /* The current version  */

extern char *LibraryName;       /* TODO : What is this? */
extern char *LibraryPointer     /* TODO : What is this? */;
extern char *ArgumentsName;     /* TODO : What is this? */
extern char *EnvironmentName;   /* TODO : What is this? */

extern char* Home;              /* The home directory of the current user */

extern int Syntax;              /* Which parser to use */
extern int MemorySize;          /* Heap size */

extern int DebugMutex;          /* Turns on debugging for lock/unlock */
extern int Debugging;           /* If we are printing debugging statements*/
extern int TraceBack;           /* TODO : I don't know */
extern int StackDebugging;      /* Outputs the stack */
extern FILE* DebugFile;         /* File for output from debugging */

extern int GCMode;              /* Which garbage collector are we using */
extern int GCCount;             /* How many garbage collections performed */
extern int GCDisplay;           /* Print out garbage collection information */
extern int GCQueueCount;        /* How many threads are waiting for a Garbage Collection */
extern int QueueCount;          /* How many items are waiting for a thread */

extern int ThreadError;         /* If error in thread set to thread id of first thread to report an error */
extern int Caching;             /* Sometimes we can save computed results */
extern int ShuttingDown;        /* Tells threads to shut down or not */

extern int GlobalLock;          /* Single global lock for semaphores */

#define SCAM 1
#define SWAY 2

#endif
