
/*
 *  Main Author : John C. Lusth
 *  Barely Authors : Jeffrey Robinson, Gabriel Loewen
 *  Last Edit : May 4, 2014
 *
 *  Helper functions.  
 *
 */


#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "parser.h"

#include "scam.h"
#include "util.h"

int
Fatal(char *fmt, ...)
    {
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    QueueCount = 0;
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

/* only -oXXX  or -o XXX options */

int
ProcessOptions(int argc, char **argv)
{
    int argIndex;
    int argUsed;
    int separateArg;
    char *arg;

    argIndex = 1;

    while (argIndex < argc && *argv[argIndex] == '-')
        {

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
            case 'g':
                {
                GCDisplay = !GCDisplay;
                }
                break;
            case 'G':
                {
                if (!strcmp(arg, "sc"))
                    {
                    GCMode = STOP_AND_COPY;
                    argUsed = 1;
                    }
                else if (!strcmp(arg, "ms"))
                    {
                    GCMode = MARK_SWEEP;
                    argUsed = 1;
                    }
                else
                    {
                    GCMode = DEFAULT_GC;
                    }
                }
                break;
            case 'i':
                {
                Syntax = SWAY;
                }
                break;
            case 'M':
                {
                printf("MemorySize = %d\n",MemorySize);
                exit(0);
                }
                break;
            case 'm':
                {
                MemorySize *= atof(arg);
                argUsed = 1;
                }
                break;
            case 't':
                {
                TraceBack = 1;
                }
                break;
            case 'v':
                printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
                exit(0);
                break;
            /* disable printing out addresses, for debugging */
            case 'd':
                {
                Debugging = 1;
                }
                break;
            case 'D':
                {
                StackDebugging = 1;
                }
                break;
            case 'h':
                {
                    printf("scam2 <options> <FILE>\n");
                    printf("-------------------------------------------------\n");
                    printf("\td - Output debugging statements if error\n\n");
                    printf("\tD - Print stack modifications\n\n");
                    printf("\tg - Print GC statistics\n\n");
                    printf("\tG - Select the garbage collector, -G<NAME>\n");
                    printf("\t    Current options are\n");
                    printf("\t    sc - Stop and Copy (default)\n");
                    printf("\t    ms - Mark-sweep-compact\n\n");
                    printf("\th - print this menu\n\n");
                    printf("\ti - Use Sway interpreter\n\n");
                    printf("\tM - Print out the default memory size.\n\n");
                    printf("\tm - Set memory size in bytes, -m<SIZE>\n\n");
                    printf("\tt - Enable traceback\n\n");
                    printf("\tv - Print out the version\n\n");
                    exit(0);
                }
                break;
            default:
                Fatal("option %s not understood\n",argv[argIndex]);
            }

        if (separateArg && argUsed)
        {
            ++argIndex;
        }

        ++argIndex;
        }

    return argIndex;
}
