#include <stdio.h>

extern int ProcessOptions(int,char **);      /* command line option handler */
extern int Fatal(char *,...);
extern FILE * OpenFile(char *,char *);
extern void Pause(void);
extern void *New(int);
extern void *ReNew(void *,int);
extern char *StringDup(char *);
