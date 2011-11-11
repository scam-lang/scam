typedef int (*PRIM)(int);
extern PRIM BuiltIns[];
extern void loadBuiltIns(int);
extern void installArgsEnv(int,int,char **,char **,int);
