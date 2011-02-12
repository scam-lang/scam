/*
 * useful functions
 *
 * written by John C. Lusth
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "util.h"

extern char *PROGRAM_NAME;
extern char *PROGRAM_VERSION;

int
Fatal(char *fmt, ...)
    {
    va_list ap;

    //printf("encountered a fatal error...\n");

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    exit(-1);

    return -1;
    }

FILE *
OpenFile(char *fileName,char *mode)
    {
    FILE *fp;

    if ((fp = fopen(fileName, mode)) == 0)
        {
        char *s;
        switch (*mode)
           {
           case 'w':
               s = "writing";
               break;
           case 'r':
               s = "reading";
               break;
           case 'a':
               s = "appending";
               break;
           default:
               s = 0;
               break;
           }
        if (s)
            Fatal("file %s could not be opened for %s\n", fileName, s);
        else
            Fatal("file %s could not be opened mode %s\n", fileName, mode);
        }

    return fp;
    }

char *
StringDup(char *src)
    {
    char *tar = (char *) New(sizeof(char) * (strlen(src) + 1));

    strcpy(tar,src);

    return tar;
    }

/* only -oXXX  or -o XXX options */

int
ProcessOptions(int argc, char **argv)
{
    int argIndex;
    int argUsed;
    int separateArg;
    char *arg;

    argIndex = 1;

    while (argIndex < argc && *argv[argIndex] == '-') {

        separateArg = 0;
        argUsed = 0;

        if (argv[argIndex][2] == '\0')
            {
            arg = argv[argIndex+1];
            separateArg = 1;
            }
        else
            arg = argv[argIndex]+2;

        switch (argv[argIndex][1])
            {
            /*
             * when option has an argument, do this
             *
             *     examples are -m4096 or -m 4096
             *
             *     case 'm':
             *         MemorySize = atol(arg);
             *         argUsed = 1;
             *         break;
             *
             *
             * when option does not have an argument, do this
             *
             *     example is -a
             *
             *     case 'a':
             *         PrintActions = 1;
             *         break;
             */
            case 'm':
                {
                extern int MemorySize;
                MemorySize = atoi(arg);
                argUsed = 1;
                }
                break;
            case 'v':
                printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
                exit(0);
                break;
            default:
                Fatal("option %s not understood\n",argv[argIndex]);
            }

        if (separateArg && argUsed)
            ++argIndex;

        ++argIndex;
        }

    return argIndex;
}

void *
New(int size)
    {
    void *p = malloc(size);
    if (p == 0)
        Fatal("out of memory\n");

    return p;
    }

void *
ReNew(void *q, int size)
    {
    void *p = realloc(q,size);
    if (p == 0)
        Fatal("out of memory\n");

    return p;
    }
