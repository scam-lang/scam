extern int eval(int,int);
extern int evalCall(int,int,int);
extern int evalList(int,int,int);
extern int evalExprList(int,int);
extern int makeRun(int (*f)(int,int),int);

#define NORMAL 1
#define NO_EVALUATION 2
#define STRAIGHT 3

#define ALL 3
#define ALLBUTLAST 4
