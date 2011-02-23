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

char *PROGRAM_NAME = "scam";
char *PROGRAM_VERSION = "0.1";
int displayPrimitives = 0;
int displayHelp = 0;

int 
main(int argc,char **argv,char **envv)
    {
    int argIndex;
    int ptree,env;
    int result;
    PARSER *p;

    argIndex = ProcessOptions(argc, argv);

    //if (argc-argIndex > 1)
        //Fatal(USAGE_ERROR,"usage: %s [input_file]\n", PROGRAM_NAME);

    memoryInit(0);

    p = newParser(argv[argIndex]);
    ptree = parse(p);

    env = makeEnvironment(0,0,0,0);

    loadBuiltIns(env);

    //debug("global env: ",env);

    env = makeEnvironment(env,0,0,0);

    //debug("parse tree",ptree);

    push(env);
    push(ptree);

    result = eval(ptree,env);

    pop();
    pop();

    if (isThrow(result))
        {
        printf("throw!\n");
        while (error_trace(result) != 0)
            {
            printf("file %s, line %d\n",
                SymbolTable[ival(error_file(result))],ival(error_line(result)));
            result = error_trace(result);
            }
        printf("%s\n",cellString(0,0,error_message(result)));
        }
    else
        {
        pp(stdout,result);
        printf("\n");
        }

    return 0;
    }
