
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "types.h"
#include "env.h"
#include "cell.h"
#include "eval.h"
#include "sem.h"
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include <semaphore.h>
#include <sched.h>
 
int sharedSize = 1;
int controlSize = 1;
int sharedMemoryAllocated = 0;
CELL *shared;
int sharedID;
int semaphoreID;
sem_t *semaphore;

static int semaphoreDebugging = 0;

/* (allocateSharedMemory) */

int
allocateSharedMemory(int args)
    {
    int i;

    //printf("shared size is %d\n", sharedSize);
    //printf("control size is %d\n", controlSize);

    /* allocate one more for the error code */
    sharedID = shmget(IPC_PRIVATE,
        (sharedSize + controlSize + 1) * sizeof(CELL),S_IRUSR|S_IWUSR);
    semaphoreID = shmget(IPC_PRIVATE,sizeof(sem_t),S_IRUSR|S_IWUSR);
 
    if (sharedID < 0 || semaphoreID < 0)
        {
        return throw(parallelExceptionSymbol,
            "failed to allocate semaphore and shared memory of size %d",
            sharedSize + controlSize);
        }

    shared = (CELL *) shmat(sharedID,(void *) 0,0);
    semaphore = (sem_t *) shmat(semaphoreID,(void *) 0,0);
 
    if (shared == (CELL *) -1 || semaphore == (sem_t *) -1)
        {
        return throw(parallelExceptionSymbol,
            "failed to attach semaphore or shared memory");
        }
 
    /* initialize the shared memory */

    for (i = 0; i < sharedSize + controlSize + 1; ++i)
        {
        shared[i].type = INTEGER;
        shared[i].ival = 0;
        }
    shared[0].ival = -1; //set error code to -1 (no child erred)
    sem_init(semaphore,1,1);

    sharedMemoryAllocated = 1;

    return 0;
    }

/* (freeSharedMemory) */

int
freeSharedMemory(int args)
    {
    if (sem_destroy(semaphore) == -1)
        {
        return throw(parallelExceptionSymbol,
            "failed to destroy semaphore");
        }

    if ((shmdt(shared)) == -1 || shmdt(semaphore) == -1)
        {
        return throw(parallelExceptionSymbol,
            "failed to detach shared memory or semaphore");
        }

    if ((shmctl(sharedID, IPC_RMID, (void *) 0)) == -1
    ||  (shmctl(semaphoreID, IPC_RMID, (void *) 0)) == -1)
        {
        return throw(parallelExceptionSymbol,
            "failed to free shared memory"
            );
        }

    sharedMemoryAllocated = 0;

    return 0;
    }

/* (convertSharedMemory) */

int
convertSharedMemory(int args)
    {
    int i,attach,spot,result;

    /* convert shared memory to an array */

    assureMemory("convertSharedMemory",sharedSize * 2,&args,(int *)0);

    result = 0;
    attach = 0;
    for (i = 0; i < sharedSize; ++i)
        {
        spot = ucons(0,0);
        type(spot) = ARRAY;
        count(spot) = sharedSize - i;
        if (i == 0)
            result = spot;
        else
            cdr(attach) = spot;
        attach = spot;
        }

    for (i = 0; i < sharedSize; ++i)
        {
        spot = ucons(0,0);
        /* first slot is reserved for an error code */
        the_cars[spot] = shared[i+controlSize+1];
        car(result + i) = spot;
        }

    return result;
    }

/* (spexecute # $) */

int
spexecute(int args)
    {
    int env;
    int result;
    int spot;
 
    env = car(args);

    spot = cadr(args); /* skip over the environment */

    push(args);
    while (spot != 0) /* loop over lambdas */
        {
        push(env);
        push(spot);
        result = eval(car(spot),env);
        spot = pop();
        env = pop();
        rethrow(result,0);
        spot = cdr(spot);
        }
    args = pop();

    return 0;
    }

/* (pexecute # $) */

int
pexecute(int args)
    {
    int env;
    int exprs;
 
    //printf("shared size is %d\n", sharedSize);
    //printf("control size is %d\n", controlSize);

    if (!sharedMemoryAllocated)
        {
        return throw(parallelExceptionSymbol,
            "file %s,line %d: setShared: shared memory needs to be allocated\n"
            "via the allocateSharedMemory function",
            SymbolTable[file(args)],line(args));
        }

    env = car(args);
    exprs = cadr(args); /* skip over the environment */

    while (exprs != 0) /* loop over lambdas */
        {
        int pid = fork();
        if (pid == 0)
            {
            //debug("forking ",car(exprs));
            int result = eval(car(exprs),env);
            if (isThrow(result))
                shared[0].ival = getpid();
            exit(0);
            }
        else
            {
            exprs = cdr(exprs);
            }
        }

    /* wait for all the children to finish */

    exprs = cadr(args); /* skip over the environment */
    while (exprs != 0)
        {
        wait((void *) 0);
        exprs = cdr(exprs);
        }

    /* check for an error code */

    if (shared[0].ival >= 0)
        {
        int pid = shared[0].ival;
        shared[0].ival = -1;
        return throw(parallelExceptionSymbol,
            "file %s,line %d: parallel execution of process %d failed\n"
            "try using *pexecute instead of pexecute for more information",
            SymbolTable[file(args)],line(args),pid);
        }

    return 0;
    }

/* (setShared index value) */

int
setShared(int args)
    {
    int slot = ival(car(args));
    int value = cadr(args);

    if (!sharedMemoryAllocated)
        {
        return throw(parallelExceptionSymbol,
            "file %s,line %d: setShared: shared memory needs to be allocated\n"
            "via the allocateSharedMemory function",
            SymbolTable[file(args)],line(args));
        }

    if (slot < 0 || slot >= sharedSize)
        return throw(parallelExceptionSymbol,
            "file %s,line %d: "
            "index (%d) into shared memory is out of bounds\n",
            SymbolTable[file(args)],line(args),
            slot);

    /* first slot is reserved for an error code */
    shared[slot+controlSize+1] = the_cars[value];

    return value;
    }

/* (getShared index) */

int
getShared(int args)
    {
    int slot = ival(car(args));

    if (!sharedMemoryAllocated)
        {
        return throw(parallelExceptionSymbol,
            "file %s,line %d: getShared: shared memory needs to be allocated\n"
            "via the allocateSharedMemory function",
            SymbolTable[file(args)],line(args));
        }

    if (slot < 0 || slot >= sharedSize)
        return throw(parallelExceptionSymbol,
            "file %s,line %d: "
            "index (%d) into shared memory is out of bounds\n",
            SymbolTable[file(args)],line(args),
            slot);

    int result = ucons(0,0);
    /* first slot is reserved for an error code */
    the_cars[result] = shared[slot+controlSize+1];
    return result;
    }

/* (setSharedSize size) */

int
setSharedSize(int args)
    {
    int old = sharedSize;
    sharedSize = ival(car(args));
    return newInteger(old);
    }

/* (getSharedSize) */

int
getSharedSize(int args)
    {
    return newInteger(sharedSize);
    }

/* (yield) */

int
yield(int args)
    {
    sched_yield();
    sched_yield();
    return 0;
    }

/* (debugSemaphore mode) */

int
debugSemaphore(int args)
    {
    if (!sharedMemoryAllocated)
        {
        return throw(parallelExceptionSymbol,
            "file %s,line %d: debugSemaphore: "
            "shared memory needs to be allocated\n"
            "via the allocateSharedMemory function",
            SymbolTable[file(args)],line(args));
        }

    semaphoreDebugging = car(args);
    return 0;
    }

/* (getPID) */

int
getPID(int args)
    {
    return newInteger(getpid());
    }

/* (acquire) */

int
acquire(int args)
    {
    if (semaphoreDebugging)
        fprintf(stderr,"process %d is acquiring...\n",getpid());
    sem_wait(semaphore);
    if (semaphoreDebugging)
        fprintf(stderr,"process %d has acquired.\n",getpid());
    return 0;
    }

/* (release) */

int
release(int args)
    {
    if (semaphoreDebugging)
        fprintf(stderr,"process %d is releasing...\n",getpid());
    sem_post(semaphore);
    if (semaphoreDebugging)
        fprintf(stderr,"process %d has released.\n",getpid());
    return 0;
    }

/* (setControlSize size) */

int
setControlSize(int args)
    {
    int old = controlSize;
    controlSize = ival(car(args));
    return newInteger(old);
    }

/* (getControlSize) */

int
getControlSize(int args)
    {
    return newInteger(controlSize);
    }

/* (setControl index value)  */

int
setControl(int args)
    {
    int slot = ival(car(args));
    int value = cadr(args);

    if (!sharedMemoryAllocated)
        {
        return throw(parallelExceptionSymbol,
            "file %s,line %d: setControl: shared memory needs to be allocated "
            "via the allocateSharedMemory function",
            SymbolTable[file(args)],line(args));
        }

    if (slot < 0 || slot >= controlSize)
        return throw(parallelExceptionSymbol,
            "file %s,line %d: "
            "index (%d) into control memory is out of bounds\n",
            SymbolTable[file(args)],line(args),
            slot);

    /* first slot is reserved for an error code */
    shared[slot + 1] = the_cars[value];

    return value;
    }

/* (getControl index) */

int
getControl(int args)
    {
    int slot = ival(car(args));

    if (!sharedMemoryAllocated)
        {
        return throw(parallelExceptionSymbol,
            "file %s,line %d: getControl: shared memory needs to be allocated "
            "via the allocateSharedMemory function",
            SymbolTable[file(args)],line(args));
        }

    if (slot < 0 || slot >= controlSize)
        return throw(parallelExceptionSymbol,
            "file %s,line %d: "
            "index (%d) into control memory is out of bounds\n",
            SymbolTable[file(args)],line(args),
            slot);

    int result = ucons(0,0);
    /* first slot is reserved for an error code */
    the_cars[result] = shared[slot+1];
    return result;
    }
