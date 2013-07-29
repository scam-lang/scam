#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include "cell.h"
 
extern CELL *shared;
extern int throw(int,char *,...);
extern int eval(int,int);
 
int
pexecute(int args)
    {
    int env;
    int size;
    int sharedID;
    int result;
 
    env = car(args);
    size = ival(cadr(args)) * sizeof(CELL);

    sharedID = shmget(IPC_PRIVATE,size,S_IRUSR|S_IWUSR);
 
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

    result = eval(car(args),env); /* run the cleanup routine */

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
