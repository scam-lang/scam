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

extern void ppf(char *,int,char *);

#define ENV_HOME "HOME="

extern int nextStackPtr;
extern FILE *Input;

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "scam";
char *PROGRAM_VERSION = "1.6b";
int displayPrimitives = 0;
int displayHelp = 0;
int TraceBack = 0;
int Syntax = SCAM;

char *LibraryName = "SCAM_LIB";
char *LibraryPointer = "/usr/local/lib/scam/";
char *ArgumentsName = "ScamArgs";
char *EnvironmentName = "ScamEnv";
char *Home = "~";

static int addToEnvironment(int,char *,int );

int 
main(int argc,char **argv,char **envv)
    {
    int argIndex;
    int i;
    int env;
    int result;

    //printf("cell size: %d\n",sizeof(CELL));
    argIndex = ProcessOptions(argc, argv);

    if (argc-argIndex == 0)
        {
        Fatal("usage: %s INPUT_FILE [ARGUMENTS]\n", PROGRAM_NAME);
        }

    /* initialize memory */

    memoryInit(0);

    /* find the home directory for finding files */

    for (i = 0; envv[i] != 0; ++i)
        if (strstr(envv[i],ENV_HOME) == envv[i]) 
            Home = strdup(envv[i]+strlen(ENV_HOME));

    /* global level of environment */

    env = makeEnvironment(0,0,0,0);

    loadBuiltIns(env);

    installArgsEnv(argIndex,argc,argv,envv,env);

    //debug("global env: ",env);

    /* main.lib level of environment */

    env = makeEnvironment(env,0,0,0);
    result = addToEnvironment(env,"main.lib",SCAM);
    if (isThrow(result)) goto ERROR;

    /* add in Sway compatibility layer, if necessary */

    if (Syntax == SWAY)
        {
        env = makeEnvironment(env,0,0,0);
        result = addToEnvironment(env,"sway.lib",SWAY);
        if (isThrow(result)) goto ERROR;
        }

    /* user level of environment */

    env = makeEnvironment(env,0,0,0);
    result = addToEnvironment(env,argv[argIndex],Syntax);
    if (isThrow(result)) goto ERROR;

    return 0;

ERROR:
    //int last;
    //debug("thrown",result);
    if (TraceBack)
        {
        int spot = error_trace(result);
        //int prettySetup = ucons(prettyStatementSymbol,ucons(ucons(quoteSymbol,
        //       ucons(0,0)),ucons(newString("    "),0)));
        printf("EXCEPTION TRACE --------------------\n");
        while (spot != 0)
            {
            fprintf(stdout,"from %s,line %d:\n    ",
                filename(spot),line(spot));
            ppf("",car(spot),"\n");
            //car(cdr(car(cdr(prettySetup)))) = car(spot);
            //presult = eval(prettySetup,env);
            //if (isThrow(presult))
            //    {
            //    ppf("pretty print error: ",error_value(result),"\n");
            //    break;
            //    }
            spot = cdr(spot);
            }
        }
    printf("------------------------------------\n");
    debug("EXCEPTION",error_code(result));
    scamPP(stdout,error_value(result));
    printf("\n");

    return -1;
    }

static int
addToEnvironment(int env,char *fileName,int mode)
    {
    int ptree,result,s;
    PARSER *p;
    char buffer[1024];

    p = newParser(fileName);
    if (p == 0)
        Fatal("file %s could not be opened for reading\n",fileName);

    // indicate that file has already been processed

    assureMemory("addToEnvironment",DEFINE_CELLS + 1,&env,(int *)0);
    snprintf(buffer,sizeof(buffer),"__included_%s",fileName);
    s = newSymbol(buffer);
    defineVariable(env,s,trueSymbol);

    // now parse the file

    push(env);
    if (mode == SCAM)
        ptree = scamParse(p);
    else
        ptree = swayParse(p);
    env = pop();

    freeParser(p);

    rethrow(ptree,0);

    /* now evaluate the file */

    push(env);
    result = eval(ptree,env);
    env = pop();

    return result;
    }
