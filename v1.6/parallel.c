#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include "types.h"
#include "cell.h"
 
extern CELL *shared;
extern int sharedSize;
extern int throw(int,char *,...);
extern int eval(int,int);
 
int
pexecute(int args)
    {
    int env;
    int size;
    int sharedID;
    int result;
    int i,spot,attach;
 
    env = car(args);
    size = ival(cadr(args)) * sizeof(CELL);
    if (size > 0) sharedSize = size;

    sharedID = shmget(IPC_PRIVATE,sharedSize,S_IRUSR|S_IWUSR);
 
    if (sharedID < 0)
        {
        return throw(exceptionSymbol,
            "file %s,line %d: failed to allocate shared memory",
            SymbolTable[file(car(args))],line(car(args)));
        }

    shared = (CELL *) shmat(sharedID,(void *) 0,0);
 
    if (shared == (CELL *) -1)
        {
        return throw(exceptionSymbol,
            "file %s,line %d: failed to attach shared memory",
            SymbolTable[file(car(args))],line(car(args)));
        }
 
    /* initialize the shared memory */

    for (i = 0; i < sharedSize; ++i)
         {
         shared[i].type = INTEGER;
         shared[i].ival = 0;
         }

    args = cddr(args); /* skip over the environment and the size */

    while (cdr(args) != 0) /* loop over lambdas, except the last */
        {
        int pid = fork();
        if (pid == 0)
            {
            result = eval(car(args),env);
            exit(0); //what if the child throws an exception?
            }
        else
            args = cdr(args);
        }

    wait((void *) 0); // will this wait for all children?

    /* convert shared memory to an array */

    assureMemory("pexecute",sharedSize,(int *)0);

    result = 0;
    attach = 0;
    for (i = 0; i < sharedSize; ++i)
        {
        spot = ucons(0,0);
        the_cars[spot] = shared[i];
        type(spot) = ARRAY;
        count(spot) = sharedSize - i;
        if (i == 0)
            result = spot;
        else
            cdr(attach) = spot;
        attach = spot;
        }

    if ((shmdt(shared)) == -1)
        {
        return throw(exceptionSymbol,
            "file %s,line %d: failed to detach shared memory",
            SymbolTable[file(car(args))],line(car(args)));
        }

    if ((shmctl(sharedID, IPC_RMID, NULL)) == -1)
        {
        return throw(exceptionSymbol,
            "file %s,line %d: failed to free shared memory",
            SymbolTable[file(car(args))],line(car(args)));
        }

    return result;
    }

/* (sharedSet index value) */

int
sharedSet(int args)
    {
    int slot = ival(car(args));
    int value = cadr(args);

    shared[slot] = the_cars[value];

    return value;
    }

/* (sharedGet index) */

int
sharedGet(int args)
    {
    int index = ival(car(args));
    int result = ucons(0,0);
    the_cars[result] = shared[index];
    return result;
    }

