/*
 *  Main Author : Jeffrey Robinson
 *  Barely Authors : John C. Lusth, Gabriel Loewen
 *  Last Edit : May 4, 2014
 *
 *  The layout of this document is such:
 *      Includes
 *          Just pthread.h
 *
 *      extern global variables
 *
 *      external functions
 *
 *      #defines
 *          Locking/Unlocking
 */


#ifndef __THREAD_H__
#define __THREAD_H__

/* pthread_t, pthread_mutex_t, pthread_key_t */
#include <pthread.h>

typedef struct threadobj
    {
    int env;
    int expr;
    int tid;
    } THREAD_ARGS;

typedef struct thread_data
{
    int tid;
    pthread_t handle;

    int *Stack;
    int StackSpot;

    char **ShadowStack;
    int ShadowSpot;

} T_DATA;


/* Thread specific functions */
extern int thread(int);
extern int tjoin(int);
extern int getTID(int);
extern int waitFor(int);
extern int lock(int);
extern int unlock(int);
extern int debugMutex(int);



extern void threadingInit();
extern void threadingShutdown();

extern void checkDeadlock(char*,int);
extern int getRawThreadIndex();

extern int validate(char *);

/* Number of threads working and created */
extern THREAD_ARGS ThreadQueue[];
extern int WorkingThreads;
extern int CreatedThreads;
extern int QueueCount;

/* Deadlock detection */
extern int *P_REQ;
extern int *P_P_REQ;
extern int *T_P_REQ;

/* Thread handles, mutexs, keys, and conditions */
//extern T_DATA Thread[];
extern pthread_t *Thread;
extern pthread_mutex_t mutex,p_mutex,t_mutex;
extern pthread_key_t key;

#define MAX_THREADS 128 
/* One of these will be faster */
#define __THREAD_ID (getRawThreadIndex())
#define THREAD_ID (*((int*)pthread_getspecific(key)))

/* Deadlock detection, lock interests */
#define NO_INTEREST 0
#define ACQUIRED 1
#define REQUESTED 2

/* Memory locks */
#define T_P()                           \
do                                      \
    {                                   \
    int S_I = THREAD_ID;                \
    T_P_REQ[S_I] = REQUESTED;           \
    checkDeadlock(__FILE__,__LINE__);   \
    pthread_mutex_lock(&t_mutex);       \
    T_P_REQ[S_I] = ACQUIRED;            \
    } while(0)

#define T_V()                           \
do                                      \
    {                                   \
    T_P_REQ[THREAD_ID] = NO_INTEREST;   \
    pthread_mutex_unlock(&t_mutex);     \
    } while(0)

/* Printing locks */
#define P_P()                                               \
do                                                          \
    {                                                       \
    int S_I = THREAD_ID;                                    \
    if( P_P_REQ[S_I] != NO_INTEREST)                        \
        {                                                   \
        Fatal("Thread %d : Double lock P_P (%s:%d)\n",S_I,  \
                __FILE__,                                   \
                __LINE__);                                  \
        }                                                   \
    else                                                    \
        {                                                   \
        if(0 && StackDebugging)                             \
            {                                               \
            fprintf(stderr,                                 \
                    "P_P: Thread %d (%s,%d)\n",             \
                    THREAD_ID,                              \
                    __FILE__,                               \
                    __LINE__);                              \
            }                                               \
        P_P_REQ[S_I] = REQUESTED;                           \
        checkDeadlock(__FILE__,__LINE__);                   \
        pthread_mutex_lock(&p_mutex);                       \
        P_P_REQ[S_I] = ACQUIRED;                            \
        }                                                   \
    } while(0)

#define P_V()                                               \
do                                                          \
    {                                                       \
    int S_I = THREAD_ID;                                    \
    if(P_P_REQ[S_I] != ACQUIRED)                            \
        {                                                   \
        printf("Thread %d tried to unlock P_V\n", S_I);     \
        }                                                   \
    else                                                    \
        {                                                   \
        if(0 && StackDebugging)                             \
            {                                               \
            fprintf(stderr,                                 \
                    "P_V: Thread %d (%s,%d)\n",             \
                    THREAD_ID,                              \
                    __FILE__,                               \
                    __LINE__);                              \
            }                                               \
        P_P_REQ[S_I] = NO_INTEREST;                         \
        pthread_mutex_unlock(&p_mutex);                     \
        }                                                   \
    } while(0)

/* Global Locks */

#define P()                                                 \
do                                                          \
    {                                                       \
    /*printf("Calling P from (%s,%d)\n", __FILE__, __LINE__); */ \
    int S_I = THREAD_ID;                                    \
    if(P_REQ[S_I]!=NO_INTEREST)                             \
        {                                                   \
        Fatal("Thread %d : Double locked P (%s:%d)\n",      \
                S_I,                                        \
                __FILE__,                                   \
                __LINE__);                                  \
        }                                                   \
    else                                                    \
        {                                                   \
        if (0 && StackDebugging)                            \
            {                                               \
            fprintf(stderr,                                 \
                    "P: Thread %d (%s,%d)\n",               \
                    THREAD_ID,                              \
                    __FILE__,                               \
                    __LINE__);                              \
            }                                               \
        P_REQ[S_I] = REQUESTED;                             \
        checkDeadlock(__FILE__,__LINE__);                   \
        pthread_mutex_lock(&mutex);                         \
        P_REQ[S_I] = ACQUIRED;                              \
        }                                                   \
    } while(0)

#define V()                                                 \
do                                                          \
    {                                                       \
    /*printf("Calling V from (%s,%d)\n", __FILE__, __LINE__); */ \
    int S_I = THREAD_ID;                                    \
    if(P_REQ[S_I]!=ACQUIRED)                                \
        {                                                   \
        printf("Thread %d unlocking unowned lock V\n",S_I); \
        printf("File = %s, Line = %d\n",__FILE__,__LINE__); \
        }                                                   \
    else                                                    \
        {                                                   \
        if (0 && StackDebugging)                            \
            {                                               \
            fprintf(stderr,                                 \
                    "V: Thread %d (%s,%d)\n",               \
                    THREAD_ID,                              \
                    __FILE__,                               \
                    __LINE__);                              \
            }                                               \
        P_REQ[THREAD_ID] = NO_INTEREST;                     \
        pthread_mutex_unlock(&mutex);                       \
        }                                                   \
    } while(0)

#endif
