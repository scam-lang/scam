/*
 * Xcheme Interpreter
 *
 * written by John C. Lusth
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "cell.h"
#include "types.h"
#include "parser.h"
#include "eval.h"
#include "env.h"
#include "prim.h"
#include "pp.h"
#include "util.h"

extern int nextStackPtr;
extern FILE *Input;

/* change PROGRAM_NAME and PROGRAM_VERSION appropriately */

char *PROGRAM_NAME = "xcheme";
char *PROGRAM_VERSION = "0.0";
int displayPrimitives = 0;
int displayHelp = 0;

int 
main(int argc,char **argv,char **envv)
    {
    int argIndex;
    int ptree,env;
    int result;

    argIndex = ProcessOptions(argc, argv);

    //if (argc-argIndex > 1)
        //Fatal(USAGE_ERROR,"usage: %s [input_file]\n", PROGRAM_NAME);

    memoryInit(0);

    parserInit(argv[argIndex]);
    ptree = parse();

    env = makeObject(0,0,0,0);

    loadBuiltIns(env);

    fprintf(stdout,"global env: ");
    pp(stdout,env);
    fprintf(stdout,"\n");

    env = makeObject(env,0,0,0);

    fprintf(stdout,"parse tree: ");
    pp(stdout,ptree);
    fprintf(stdout,"\n");

    result = eval(ptree,env);

    pp(stdout,result);
    printf("\n");

    return 0;
    }
