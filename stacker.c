#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"

#define VERBOSE 0

int
main(int argc,char **argv)
    {
    FILE *fp = fopen(argv[1],"r");
    char *location,*mode,*item;
    char **stack;
    int i;
    int size = 512;
    int count = 0;
    int line = 0;

    stack = allocate(sizeof(char *) * size);

    location = readToken(fp);
    while (!feof(fp))
        {
        ++line;
        if (VERBOSE) printf("%s ",location);
        mode = readToken(fp);
        if (strcmp(mode,"pushing:") == 0)
            {
            if (count == size)
                {
                size *= 2;
                stack = reallocate(stack,sizeof(char *) * size);
                }
            item = readLine(fp);
            if (VERBOSE) printf("pushing %s\n",item);
            stack[count++] = item;
            }
        else if (strcmp(mode,"popping:") == 0)
            {
            item = readLine(fp);
            if (VERBOSE) printf("popping %s\n",item);
            if (strcmp(item,stack[count-1]) != 0)
                 {
                 printf("WARNING, line %d: popping %s, was %s\n",
                     line,item,stack[count-1]);
                 getchar();
                 }
            free(stack[count-1]);
            free(item);
            --count;
            }
        else if (strcmp(mode,"replacing:") == 0)
            {
            int index = readInt(fp);
            item = readLine(fp);
            free(stack[count-index-1]);
            stack[count-index-1] = item;
            if (VERBOSE) printf("replacing %d %s\n",index,item);
            }
        else if (strcmp(mode,"updating:") == 0)
            {
            int index = readInt(fp);
            item = readLine(fp);
            free(stack[count-index-1]);
            stack[count-index-1] = item;
            if (VERBOSE) printf("updating %d %s\n",index,item);
            }
        else if (strcmp(location,"gc") == 0)
            {
            item = readLine(fp);
            printf("gc %s%s\n",mode,item);
            free(item);
            }
        else
            {
            item = readLine(fp);
            if (VERBOSE) printf("%s%s\n",mode,item);
            free(item);
            }
        free(mode);
        free(location);
        location = readToken(fp);
        }

    for (i = 0; i < count; ++i)
        printf("%d: %s\n",i,stack[i]);

    return 0;
    }
