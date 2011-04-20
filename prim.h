typedef int (*PRIM)(int);
extern PRIM BuiltIns[];
extern void loadBuiltIns(int);
extern void installArgsEnv(int,char **,char **,int);
