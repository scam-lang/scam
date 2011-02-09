#include <stdio.h>

extern int ProcessOptions(int,char **);      /* command line option handler */
extern void Fatal(const char *,...);
extern FILE * OpenFile(const char *,const char *);
extern void Pause(void);
extern void *New(int);
extern void *ReNew(void *,int);
extern char *StringDup(const char *);
