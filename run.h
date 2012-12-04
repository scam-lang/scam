int evalR(int,int);
int analyze(int);
int analyzeCall(int);
int analyzeArgs(int);
extern int makeRun(int (*f)(int,int),int);
extern int runIdentity(int,int);
extern int runLookup(int,int);
extern int runCall(int,int);

#define NORMAL 1
#define NO_EVALUATION 2

#define ALL 3
#define ALLBUTLAST 4
