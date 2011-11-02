/*
 * SCAM Interpreter
 *
 * written by John C. Lusth
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "cell.h"
#include "types.h"
#include "parser.h"
#include "eval.h"
#include "env.h"
#include "prim.h"
#include "pp.h"
#include "util.h"

#define ENV_HOME "HOME="

extern int nextStackPtr;
extern FILE *Input;

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "scam";
char *PROGRAM_VERSION = "0.1.9";
int displayPrimitives = 0;
int displayHelp = 0;
int TraceBack = 0;

char *LibraryName = "SCAM_LIB";
char *LibraryPointer = "/usr/local/lib/scam/";
char *ArgumentsName = "ScamArgs";
char *EnvironmentName = "ScamEnv";
char *Home = "~";

int 
main(int argc,char **argv,char **envv)
    {
    int argIndex;
    int i;
    int ptree,env,s;
    int result;
    char buffer[512];
    PARSER *p;

    argIndex = ProcessOptions(argc, argv);

    if (argc-argIndex == 0)
	{
        Fatal("usage: %s INPUT_FILE [ARGUMENTS]\n", PROGRAM_NAME);
	}

    memoryInit(0);

    /* find the home directory for finding files */

    for (i = 0; envv[i] != 0; ++i)
        if (strstr(envv[i],ENV_HOME) == envv[i]) 
            Home = strdup(envv[i]+strlen(ENV_HOME));

    /* global level of environment */

    env = makeEnvironment(0,0,0,0);

    loadBuiltIns(env);

    installArgsEnv(argc,argv,envv,env);

    //debug("global env: ",env);

    /* main.lib level of environment */

    env = makeEnvironment(env,0,0,0);

    p = newParser("main.lib");
    if (p == 0)
        Fatal("file main.lib could not be opened for reading\n",argv[argIndex]);

    // indicate that main.lib has already been processed

    assureMemory("scam:main.lib",DEFINE_CELLS + 1,&env,&ptree,0);
    s = newSymbol("__included_main.lib");
    defineVariable(env,s,trueSymbol);

    push(env);
    result = parse(p);
    ptree = result;
    env = pop();

    freeParser(p);

    if (isThrow(result)) goto ERROR;

    /* now evaluate main.lib */

    push(env);
    result = eval(result,env);
    env = pop();

    if (isThrow(result)) goto ERROR;

    /* user level of environment */

    env = makeEnvironment(env,0,0,0);

    p = newParser(argv[argIndex]);
    if (p == 0)
        Fatal("file %s could not be opened for reading\n",argv[argIndex]);

    // indicate that the user file has already been processed

    assureMemory("scam:user",DEFINE_CELLS + 1,&env,&ptree,0);
    snprintf(buffer,sizeof(buffer),"__included_%s",argv[argIndex]);
    s = newSymbol(buffer);
    defineVariable(env,s,trueSymbol);

    push(env);
    result = parse(p);
    ptree = result;
    env = pop();

    freeParser(p);

    if (isThrow(ptree)) goto ERROR;

    /* now evaluate the user file */

    push(env);
    result = eval(ptree,env);
    env = pop();

    if (isThrow(result)) goto ERROR;

    return 0;

ERROR:
    //int last;
    //debug("thrown",result);
    debug("EXCEPTION",error_code(result));
    pp(stdout,error_value(result));
    printf("\n");
    if (TraceBack)
        {
        int spot = error_trace(result);
        while (spot != 0)
            {
            fprintf(stdout,"   from %s,line %d: ",
                SymbolTable[file(spot)],line(spot));
            debug(0,car(spot));
            spot = cdr(spot);
            }
        }

    return -1;
    }
