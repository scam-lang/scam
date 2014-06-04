#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wait.h>
#include <sched.h>
#include <time.h>

int
main()
    {
    int pid;
    int *shared;
    int sharedID,semaphoreID;
    sem_t *semaphore;

    sharedID = shmget(IPC_PRIVATE,1,S_IRUSR|S_IWUSR);
    semaphoreID = shmget(IPC_PRIVATE,sizeof(sem_t),S_IRUSR|S_IWUSR);
 
    if (sharedID < 0)
        {
        fprintf(stderr,"failed to allocate shared memory");
        exit(-1);
        }

    shared = (int *) shmat(sharedID,(void *) 0,0);
    semaphore = (sem_t *) shmat(semaphoreID,(void *) 0,0);
 
    if (shared == (int *) -1)
        {
        fprintf(stderr,"failed to attach shared memory");
        exit(-2);
        }
 
    /* initialize the shared memory */

    *shared = 13;
    sem_init(semaphore,1,1);

    pid = fork();

    if (pid == 0) //child
        {
        int seed = time((void *) 0);
        srand(seed);
        int snooze = rand() % 3;
        printf("child, with seed %d, will sleep %d...\n",seed,snooze);
        sleep(snooze);
        printf("child: about to grab the semaphore...\n");
        sem_wait(semaphore);
        printf("child starting...\n");
        *shared = *shared + 1;
        sleep(1);
        *shared = *shared + 1;
        sleep(1);
        *shared = *shared + 1;
        printf("child finished.\n");
        sem_post(semaphore);
        exit(0);
        }
    else //parent
        {
        int seed = time((void *) 0) + 1;
        srand(seed);
        int snooze = rand() % 3;
        printf("parent, with seed %d, will sleep %d...\n",seed,snooze);
        sleep(snooze);
        printf("parent: about to grab the semaphore...\n");
        sem_wait(semaphore);
        printf("parent starting...\n");
        *shared = *shared * 2;
        sleep(1);
        *shared = *shared * 2;
        sleep(1);
        *shared = *shared * 2;
        printf("parent finished.\n");
        sem_post(semaphore);
        wait((void *) 0);
        }

    printf("shared memory was 13, now it is %d\n",*shared);

    printf("\n");

    sem_destroy(semaphore);

    shmdt(semaphore);
    if ((shmdt(shared)) == -1)
        {
        fprintf(stderr,"failed to detach shared memory");
        exit(-3);
        }

    shmctl(semaphoreID,IPC_RMID,(void *) 0);
    if ((shmctl(sharedID, IPC_RMID, NULL)) == -1)
        {
        fprintf(stderr,"failed to free shared memory");
        }

    return 0;
    }

