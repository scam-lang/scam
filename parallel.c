#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
 
float *a;
float *b;
float *c;
float *d;
 
int
pexecute(int args)
    {
    int size;
    
 
pid_t pid,pid1,pid2;
pid_t cpid,cpid1,cpid2;
int status;
int shmid;
CELL *shm_ptr;
 
    size = ival(car(args)) * sizeof(CELL);

    shmid=shmget(IPC_PRIVATE,size,S_IRUSR|S_IWUSR);
 
    if (shmid < 0)
        {
        return throw(exceptionSymbol,"
            file %s,line %d: failed to allocate shared memory",
            SymbolTable[file(car(args)]),line(car(args)));
        }

    shm_ptr=(CELL *)shmat(shmid,(void *)0,0);
 
    if (shm_ptr == (float *)(-1))
{
    perror("shmat:shm_ptr");
    exit(1);
}
 
a=shm_ptr;
b=(shm_ptr+1);
c=(shm_ptr+2);
d=(shm_ptr+3);
 
 
if ((pid = fork()) == 0)
{
    *a=1+1;
    printf("a: (in Proc)= %.2f\n",*a);
    sleep(2);
    exit(0);
} else if ((pid1=fork()) == 0)
{
    *b=3+3;
    printf("b: (in Proc)= %.2f\n",*b);
    sleep(2);
    exit(0);
} else if ((pid2=fork()) == 0)
{
    *c=4+4;
    printf("c: (in Proc)= %.2f\n",*c);
    sleep(2);
    exit(0);
} else {   
     
    if ((cpid=wait(&status)) == pid)
    {
        printf("Child %d returned\n",pid);
    }
    if ((cpid1=wait(&status)) == pid1)
    {
        printf("Child %d returned\n",pid1);
    }
    if ((cpid2=wait(&status)) == pid2)
    {
        printf("Child %d returned\n",pid2);
    }
     
    printf("a: (in Parent)=%.2f\n", *a);
    printf("b: (in Parent)=%.2f\n", *b);
    printf("c: (in Parent)=%.2f\n", *c);
 
    *d=*a+*b+*c;
    printf("d=%.2f\n", *d);
 
}
 
if ((shmdt(shm_ptr)) == -1)
{
    perror("shmdt");
} else {
    if ((shmctl(shmid, IPC_RMID, NULL)) == -1)
    {
        perror("shmctl");
    }
}
 
return 0;
 
}
