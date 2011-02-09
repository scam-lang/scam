#include <stdio.h>

extern int ProcessOptions(int,char **);      /* command line option handler */
extern void Fatal(char *,...);
extern FILE * OpenFile(char *,char *);
extern void Pause(void);
extern void *New(int);
extern void *ReNew(void *,int);
extern char *StringDup(char *);
