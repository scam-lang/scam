/*
 *  Main Author : Jeffrey Robinson
 *  Contributing Authors : Gabriel Loewen
 *  Barely Authors  : John C. Lusth
 *  Last Modified : May 4, 2014
 *
 *  If multithreading is enabled a thread is started 
 *  if all other threads are busy. Otherwise the work
 *  requested goes on the work queue.  If maximum number
 *  of threads have been requested then all work goes
 *  on the work queue.
 *
 *  If multithreading is not enabled then each thread
 *  is run serially.
 *
 *  TODO : Make the main thread do work instead of busy
 *      waiting at the end of scam.c
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <sched.h>
#include <pthread.h>

#include <sys/wait.h>
#include <sys/stat.h>

#include "scam.h"
#include "cell.h"
#include "pp.h"
#include "env.h"
#include "util.h"
#include "eval.h"
#include "types.h"
#include "thread.h"

int CreatedThreads = 0;
int WorkingThreads = 0;
int ScamThreadID = 0;
int QueueCount = 0;

pthread_key_t key;
pthread_mutex_t mutex,p_mutex,t_mutex,u_mutex;

int Key[MAX_THREADS];

int *P_REQ;
int *P_P_REQ;
int *T_P_REQ;
pthread_t *Thread;

static void *dummyThread(void *);

THREAD_ARGS ThreadQueue[MAX_THREADS];
int QueueFront = 0;
int QueueBack  = 0;

/* threadingInit -  Initializes all thread specific 
                    data-structures, mutexs, and
                    thread conditionals.
 */

void
threadingInit()
    {
    pthread_key_create(&key,NULL); 

    /* Thread handles  */
    Thread = malloc(sizeof(pthread_t) * MAX_THREADS);

    /* Deadlock detection */
   
    P_REQ = malloc(sizeof(int) * MAX_THREADS);
    P_P_REQ = malloc(sizeof(int) * MAX_THREADS);
    T_P_REQ = malloc(sizeof(int) * MAX_THREADS);

    /* Thread mutexes */
    
    pthread_mutex_init(&mutex, NULL);   /* Global Mutex */
    pthread_mutex_init(&p_mutex, NULL); /* Printing Mutex*/
    pthread_mutex_init(&t_mutex, NULL); /* Memory Mutex */
    pthread_mutex_init(&u_mutex, NULL); /* User Mutex */

    /* Initialize all threading data-structures */
    int i = 0;    
    while(i < MAX_THREADS)
        {
        P_REQ[i] = NO_INTEREST;
        P_P_REQ[i] = NO_INTEREST;
        T_P_REQ[i] = NO_INTEREST;

        ThreadQueue[i].env = 0;
        ThreadQueue[i].expr = 0;
        ThreadQueue[i].tid = -1;

        /* Default each thread */
        Thread[i] = 0; //was Thread[i] = -1;
        i++;
        }

    Thread[ScamThreadID] = pthread_self();

    int *tid = malloc(sizeof(int));
    *tid = ScamThreadID;
    pthread_setspecific(key,tid);

    /* The main thread has the properties:
            working
            thread id of 0
            was created by the OS
     */
    //OUCH! ScamThreadID needs to be zero, see pre-increment in thread()
    //ScamThreadID = 1;
    CreatedThreads = 1;
    WorkingThreads = 1;
}

void
threadingShutdown()
    {
    free(pthread_getspecific(key));
    pthread_key_delete(key); 

    /* Deadlock detection */
    free(P_REQ);
    free(P_P_REQ); 
    free(T_P_REQ); 
    
    /* Thread hanles  */
   
    free(Thread);

    /* Thread mutexes */
    
    pthread_mutex_destroy(&mutex);   /* Global Mutex */
    pthread_mutex_destroy(&p_mutex); /* Printing Mutex*/
    pthread_mutex_destroy(&t_mutex); /* Memory Mutex */

    ScamThreadID = 0;
    CreatedThreads = 0;
    WorkingThreads = 0;
    }

int
lock(int args)
    {
    pthread_mutex_lock(&u_mutex);
    return 0;
    }

int
unlock(int args)
    {
    pthread_mutex_unlock(&u_mutex);
    return 0;
    }

/* thread - start up a pthread to evaluate an expression 
 *        - car(args)  := captured calling environment
 *        - cadr(args) := expression to be evaluated
 *
 *        - the calling environment gets extended, and then the expression is evaluated
 *              under the extended environment
 */

int
thread(int args)
    {
    T_P();
    ++CreatedThreads;
    ++WorkingThreads;
    P();
    ENSURE_MEMORY(MAKE_ENVIRONMENT_SIZE+INTEGER_SIZE, &args , (int *) 0);
    /* Extend the environment and create the pseudo-tid */
    int env = makeEnvironment(car(args),0,0,0);
    int expr = cadr(args);
    int tid = ++ScamThreadID;

    /* fatal if we exceed the thread limit */
    if (tid > MAX_THREADS - 1)
        {
        Fatal("You have reached the max thread limit of %d\n", MAX_THREADS);
        }

    int ret = newIntegerUnsafe(tid);

    ThreadQueue[tid].env  = env;
    ThreadQueue[tid].expr = expr;
    ThreadQueue[tid].tid  = tid;
    V();

    int *arg = malloc(sizeof(int));
    *arg = tid;
    int p_ret = pthread_create(&Thread[CreatedThreads-1], NULL, (void *)dummyThread,(void *)arg);

    T_V();
    if(p_ret)
        {
        Fatal("Error creating threads.\n");
        }

    return ret;
    }

void *
dummyThread(void *data)
    {

    /* We save our thread id into the thread specific memory location */
    pthread_setspecific(key,data);

    /* Initialize memory for this thread */
    memoryInit();

    int tid = *(int *)data;
    int result = eval(ThreadQueue[tid].expr,ThreadQueue[tid].env);

    if (isThrow(result) || isError(result))
        {
        P_P();
        if(StackDebugging)
            {
            printStack();
            }
        T_P();
        printf("Error in thread %d (%d)\n" ,tid,WorkingThreads);
        T_V();
        ShuttingDown = 1;
        QueueCount = 0;
        debug("EXCEPTION",error_code(result));
        scamPP(stdout,error_value(result));
        printf("\n");
        P_V();
        }

    T_P();
    --WorkingThreads;
    T_V();

    memoryShutdown();

    free(data);
    
    Thread[tid] = (pthread_t)NULL;

    pthread_exit(NULL);

    return NULL;
    }

int
tjoin (int args)
    {
    int tid = ival(car(args));
    pthread_join(Thread[tid], NULL);
    return ival(car(args));
    }

/* (gettid) */

int
getTID(int args)
    {
    int result;
    P();
    ENSURE_MEMORY(1,(int *) 0);
    result = newIntegerUnsafe(THREAD_ID);
    V();
    return result;
    }


int getRawThreadIndex()
    {
    int i;
    pthread_t tid;
    
    tid = pthread_self();

    i = 0;
    while(i < CreatedThreads)
        {
        if(Thread[i] == tid)
            {
            return i;
            }
        ++i;
        }
    return -1;
    }

/* getThread(int)  -   Perform a linear search for the thread id*/

int
getThreadIndex(int tid)
    {
    int i;
    i = 0;
    while(i < CreatedThreads)
        {
        if(Key[i] == tid)
            {
            return i;
            }
        ++i;
        }
    return -1;
    }


void
checkDeadlock(char* f, int l)
    {
    int T1 = 0;
    while(T1 < CreatedThreads-1)
        {
        int T2 = 0;
        while( T2 < CreatedThreads)
            {

            /* P and P_P  */
            if (    (P_REQ[T1] == ACQUIRED && P_REQ[T2] == REQUESTED) &&
                    (P_P_REQ[T1] == REQUESTED && P_P_REQ[T2] == ACQUIRED))
                {
                printf("Deadlock between threads %d and %d.",T1,T2);
                printf("Thread %d : Has P and wants P_P, Thread %d has P_P and wants P\n",T2,T2);
                }

            /* P and T_P */
            if (    (P_REQ[T1] == ACQUIRED && P_REQ[T2] == REQUESTED) &&
                    (T_P_REQ[T1] == REQUESTED && T_P_REQ[T2] == ACQUIRED))
                {
                printf("Deadlock between threads %d and %d.",T1,T2);
                printf("Thread %d : Has P and wants T_P, Thread %d has T_P and wants P\n",T2,T2);
                }
                

            /* P_P and T_P */
            if (    (P_P_REQ[T1] == ACQUIRED && P_P_REQ[T2] == REQUESTED) &&
                    (T_P_REQ[T2] == REQUESTED && T_P_REQ[T2] == ACQUIRED))
                {
                printf("Deadlock between threads %d and %d.",T1,T2);
                printf("Thread %d : Has P_P and wants T_P, Thread %d has T_P and wants P_P\n",T2,T2);
                }

            ++T2;
            }
        ++T1;
        }
    }

/*
 *  validate - Compares the types of the thread Stack with the shadow stack.
 *              Returns 0 if something is wrong, 1 otherwise.
 */

int
validate(char *str)
{
    if(StackDebugging)
        {
        int tid = THREAD_ID;
        int st_len = STACK_SPOT;
        int sh_len = SHADOW_SPOT;

        if(st_len!=sh_len)
            {
            printf("Validating at %s\n" , str );
            printf("Stacks differ in length (%d,%d) on thread %d\n",st_len,sh_len,tid);
            return 0;
            }

        int *S = STACK;

        int i = 0;
        while( i < st_len )
        {
            S_CELL C = SHADOW[i];
            int addr = S[i];

            char *shadow = C.type;
            char *light = type(addr);

            if(shadow != light)
            {
                printf("Stack for thread %d : \n",tid);
                printf("\t\tType = %s\n", type(addr));

                printf("\t\tFile = %d\n", file(addr));
                printf("\t\tLine = %d\n", line(addr));
                printf("\t\tCount = %d\n", count(addr));
                
                printf("\t\tIval = %d\n", ival(addr));
                printf("\t\tRval = %f\n", rval(addr));
                printf("\t\tFval = %p\n", fval(addr));
                
                printf("\t\tCdr = %d\n\n", cdr(addr));

                printf("Stack for thread %d : \n",tid);
                printf("\t\tAddress = %d\n", C.addr);

                printf("\t\tType = %s\n", C.type);

                printf("\t\tFile = %d\n", C.file);
                printf("\t\tLine = %d\n", C.line);
                printf("\t\tCount = %d\n", C.count);
                
                printf("\t\tIval = %d\n", C.ival);
                printf("\t\tRval = %f\n", C.rval);
                printf("\t\tFval = %p\n", C.fval);
                
                printf("\t\tCdr = %d\n\n", C.cdr);

                printf("\t\tFile Changed : %s\n", C.fileChanged);
                printf("\t\tLine Changed : %d\n\n", C.lineChanged);
                
                return 0;
            }
            ++i;
        }
    }
    return 1;
}
