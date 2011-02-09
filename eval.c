#include <stdio.h>
#include <assert.h>
#include "cell.h"
#include "types.h"
#include "parser.h"
#include "env.h"
#include "eval.h"
#include "prim.h"
#include "util.h"

static int evalCall(int,int);
static int evalBuiltIn(int,int,int);
static int processArguments(int,int,int,int);
static int thunkizedList(int,int);
static int evaluatedList(int,int);

int
eval(int expr, int env)
    {
    int t = makeThunk(expr,env);
    while (type(t) == CONS && sameSymbol(car(t),thunkSymbol))
        {
        int code,context;

        code = thunk_code(t);
        context = thunk_context(t);

        if (type(code) == INTEGER) return code;
        if (type(code) == REAL)    return code;
        if (type(code) == STRING)  return code;

        if (type(code) == SYMBOL)
            return lookupVariableValue(code,context);

        assert(type(code) == CONS);

        t = evalCall(code,context);
        }

    return t;
    }
        
static int
evalCall(int call,int env)
    {
    int closure,params,args,eargs;

    closure = eval(car(call),env);
    assert(isClosure(closure) || isBuiltIn(closure));
    params = closure_parameters(closure);
    args = cdr(call);
    eargs = processArguments(closure_name(closure),params,args,env);

    if (isBuiltIn(closure))
        {
        return evalBuiltIn(eargs,closure,env);
        }
    else
        {
        int body, xenv;
        body = closure_body(closure);
        xenv = closure_context(closure);
        xenv = makeObject(xenv,env,closure,params,eargs);
        return makeThunk(body,xenv);
        }
    }

static int
evalBuiltIn(int args,int builtIn,int env)
    {
    PRIM prim;

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
        return thunkizedList(args,env);
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
thunkizedList(args,env)
    {
    if (args == 0)
        return 0;
    else
        return cons(makeThunk(car(args),env),thunkizedList(cdr(args),env));
    }
static int
evaluatedList(args,env)
    {
    if (args == 0)
        return 0;
    else
        return cons(eval(car(args),env),evaluatedList(cdr(args),env));
    }
