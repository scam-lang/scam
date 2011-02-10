#include <stdio.h>
#include <assert.h>
#include "cell.h"
#include "types.h"
#include "parser.h"
#include "env.h"
#include "eval.h"
#include "prim.h"
#include "pp.h"
#include "util.h"

static int evalCall(int,int);
static int evalBuiltIn(int,int,int);
static int processArguments(int,int,int,int);
static int evaluatedList(int,int);
static int unevaluatedList(int);

int
eval(int expr, int env)
    {
    int t = makeThunk(expr,env);
    while (type(t) == CONS && sameSymbol(car(t),thunkSymbol))
        {
        int code,context;

        code = thunk_code(t);

        fprintf(stdout,"eval: ");
        pp(stdout,code);
        fprintf(stdout,"\n");

        context = thunk_context(t);

        if (code == 0) return 0;
        if (type(code) == INTEGER) return code;
        if (type(code) == REAL)    return code;
        if (type(code) == STRING)  return code;

        if (type(code) == SYMBOL)
            return lookupVariableValue(code,context);

        printf("eval type is %s\n",type(code));
        assert(type(code) == CONS);

        t = evalCall(code,context);
        }

    return t;
    }
        
static int
evalCall(int call,int env)
    {
    int closure,params,args,eargs;

    //printf("in evalCall: ");
    //pp(stdout,car(call));
    //printf("\n");
    closure = eval(car(call),env);
    pp(stdout,call); printf(" in evalCall\n");
    assert(isClosure(closure) || isBuiltIn(closure));
    params = closure_parameters(closure);
    args = cdr(call);
    eargs = processArguments(closure_name(closure),params,args,env);

    if (isBuiltIn(closure))
        {
        printf("call is a builtin\n");
        return evalBuiltIn(eargs,closure,env);
        }
    else
        {
        int body, xenv;
        printf("call is user defined\n");
        body = closure_body(closure);
        xenv = closure_context(closure);
        xenv = makeObject(xenv,closure,params,eargs);
        return makeThunk(body,xenv);
        }
    }

static int
evalBuiltIn(int args,int builtIn,int env)
    {
    PRIM prim;

    //printf("builtIn arguments are: ");
    //pp(stdout,args);
    //printf("\n");

    //printf("closure body is: ");
    //pp(stdout,closure_body(builtIn));
    //printf("\n");

    prim = BuiltIns[ival(closure_body(builtIn))];
    return prim(args,env);
    }

static int
processArguments(int name, int params,int args,int env)
    {
    if (params == 0 && args == 0)
        return 0;
    else if (params == 0)
        {
        Fatal("too many arguments to function %s\n",
            SymbolTable[ival(name)]);
        return 0;
        }
    else if (sameSymbol(car(params),dollarSymbol))
        return cons(makeThunk(unevaluatedList(args),env),0);
    else if (sameSymbol(car(params),atSymbol))
        return evaluatedList(args,env);
    else if (args == 0)
        {
        Fatal("too few arguments to function %s\n",
            SymbolTable[ival(name)]);
        return 0;
        }
    else if (*SymbolTable[ival(car(params))] == '$')
        {
        return cons(makeThunk(car(args),env),
            processArguments(name,cdr(params),cdr(args),env));
        }
    else
        {
        return cons(eval(car(args),env),
            processArguments(name,cdr(params),cdr(args),env));
        }
    }

static int
unevaluatedList(args)
    {
    if (args == 0)
        return 0;
    else
        return cons(car(args),unevaluatedList(cdr(args)));
    }

static int
evaluatedList(args,env)
    {
    if (args == 0)
        return 0;
    else
        return cons(eval(car(args),env),evaluatedList(cdr(args),env));
    }
