extern int eval(int,int);
extern int evalCall(int,int,int);
extern int evalList(int,int);
extern int evalListExceptLast(int,int);
extern int evalThunkList(int);
extern int evalThunkListExceptLast(int);

#define NORMAL 1
#define NO_EVALUATION 2
