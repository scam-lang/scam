
/*
 *  Main Author : John C. Lusth
 *  Barely Authors : Jeffrey Robinson, Gabriel Loewen
 *  Last Modified : May 4, 2014
 *
 * SCAM Interpreteri
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "scam.h"
#include "cell.h"
#include "types.h"
#include "parser.h"
#include "eval.h"
#include "env.h"
#include "prim.h"
#include "pp.h"
#include "util.h"

#define ENV_HOME "HOME="
#define MIN_MEMORY_SIZE 30000


/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "scam";
char *PROGRAM_VERSION = "2.0c";
int displayPrimitives = 0;
int displayHelp = 0;
int TraceBack = 0;
int Syntax = SCAM;
int GCDisplay = 1;
int GCMode = DEFAULT_GC;

int ThreadError = -1;
int Caching = 0;
int ShuttingDown = 0;

int Debugging = 0;
int StackDebugging = 0;
int DebugMutex = 0;

char *LibraryName = "SCAM_LIB";
char *LibraryPointer = "/usr/local/lib/scam/";
char *ArgumentsName = "ScamArgs";
char *EnvironmentName = "ScamEnv";
char *Home = "~";

int GlobalLock = 0;

FILE* DebugFile;


static int addToEnvironment(int,char *,int );

int 
main(int argc,char **argv,char **envv)
    {
    int argIndex;
    int i;
    int env;
    int result;
    int exitValue = 0;

    argIndex = ProcessOptions(argc, argv);

    if (MemorySize < MIN_MEMORY_SIZE)
        {
        Fatal("Desired memory size of %d cells is too small, need at least %d cells.\n",
            MemorySize,MIN_MEMORY_SIZE);
        }

    /* initialize memory */

    if (Syntax == SWAY)
        read_history(".sway_history");
    else
        read_history(".scam_history");

    scamInit(0);

    /* find the home directory for finding files */

    for (i = 0; envv[i] != 0; ++i)
        if (strstr(envv[i],ENV_HOME) == envv[i]) 
            Home = strdup(envv[i]+strlen(ENV_HOME));

    /* global level of environment */

    P();
    env = makeEnvironment(0,0,0,0);
    V();

    //printf("empty global enviroment allocated.\n");

    P();
    loadBuiltIns(env);
    V();

    //printf("built-ins loaded.\n");

    P();
    installArgsEnv(argIndex,argc,argv,envv,env);
    V();

    //printf("environment bindings loaded.\n");

    /* main.lib level of environment */

    P();
    env = makeEnvironment(env,0,0,0);
    V();

    //printf("empty library enviroment allocated.\n");
    
    result = addToEnvironment(env,"main.lib",SCAM);

    //printf("main library loaded.\n");

    if (isThrow(result)) goto ERROR;

    /* add in Sway compatibility layer, if necessary */

    if (Syntax == SWAY)
        {
        P();
        env = makeEnvironment(env,0,0,0);
        V();
        result = addToEnvironment(env,"sway.lib",SWAY);

        if (isThrow(result)) goto ERROR;
        }

    /* user level of environment */

    P();
    env = makeEnvironment(env,0,0,0);
    V();

    if (argc-argIndex == 0)
        {
        if (Syntax == SWAY)
            result = addToEnvironment(env,"sway-repl.lib",Syntax);
        else
            result = addToEnvironment(env,"repl.lib",Syntax);
        }
    else
        result = addToEnvironment(env,argv[argIndex],Syntax);

    if (Syntax == SWAY)
        write_history(".sway_history");
    else
        write_history(".scam_history");

    if (isThrow(result) && result != SHUTDOWN_THROW) goto ERROR;

    goto SHUTDOWN;

ERROR:
    ShuttingDown = 1;
    if (TraceBack)
        {
        int spot = error_trace(result);
        printf("EXCEPTION TRACE --------------------\n");
        while (spot != 0)
            {
            fprintf(stdout,"from %s,line %d:\n    ",
                filename(spot),line(spot));
            debug(0,car(spot));
            spot = cdr(spot);
            }
        }
    printf("------------------------------------\n");
    debug("EXCEPTION",error_code(result));
    scamPPFile(stdout,error_value(result));
    printf("\n");

    exitValue = -1;

SHUTDOWN:
    P_P();
    --WorkingThreads; /* Main thread is shuttig down, therefor not working */
    P_V();

    /* While there is work to be done or someone is working */

    while(!ShuttingDown && (QueueCount > 0 || WorkingThreads > 0))  /* Someone is working somewhere */
        {
        usleep(100000);
        }

    ShuttingDown = 1;
    for (i=1; i<CreatedThreads; ++i)
        {
        pthread_join(Thread[i], NULL);
        }

    scamShutdown();
    return exitValue;
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

    snprintf(buffer,sizeof(buffer),"__included_%s",fileName);
    P();
    s = newSymbolUnsafe(buffer);
    defineVariable(env,s,TrueSymbol);
    V();

    //printf("main library include tag loaded.\n");

    // now parse the file

    //printf("about to parse %s...\n",fileName);

    if (mode == SCAM)
        ptree = scamParse(p);
    else
        ptree = swayParse(p);

    //printf("parsing completed.\n");

    freeParser(p);

    rethrow(ptree);

    /* now evaluate the file */

    result = eval(ptree,env);

    //printf("after %s loaded, %d memory cells in use\n",fileName,MEMORY_SPOT);

    return result;
    }
