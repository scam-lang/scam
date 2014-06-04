extern int eval(int,int);
extern int makeRunner(int (*)(int,int),int);
extern int run(int,int);
extern int runIdentity(int,int);
extern int runLookup(int,int);
extern int runCall(int,int);
extern int runList(int,int,int);
int extractRunner(int);
int extractRunnerList(int);

#define NORMAL 1
#define NO_EVALUATION 2

#define ALL 3
#define ALLBUTLAST 4

#define RUNNER_CELLS 2
