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
static int evalBuiltIn(int,int);
static int processArguments(int,int,int,int);
static int thunkizedList(int,int);
static int evaluatedList(int,int);
static int unevaluatedList(int);

int
eval(int expr, int env)
    {
    while (1)
        {
        int t;

        //debug("eval",expr);

        if (expr == 0) return 0;
        if (type(expr) == INTEGER) return expr;
        if (type(expr) == REAL)    return expr;
        if (type(expr) == STRING)  return expr;

        if (type(expr) == SYMBOL) return lookupVariableValue(expr,env);

        //printf("eval type is %s\n",type(expr));
        assert(type(expr) == CONS);

        /* no need to assure memory here */

        t = evalCall(expr,env);

        if (!isThunk(t)) return t;

        expr = thunk_code(t);
        env = thunk_context(t);
        }

    return 0;
    }
        
static int
evalCall(int call,int env)
    {
    int closure,params,args,eargs;

    //printf("getting closure\n");
    push(env);
    push(call);
    closure = eval(car(call),env);
    call = pop();
    env = pop();
    //printf("done getting closure\n");

    //debug("evalCall",call);

    assert(isClosure(closure) || isBuiltIn(closure));
    params = closure_parameters(closure);
    args = cdr(call);

    push(closure);
    eargs = processArguments(closure_name(closure),params,args,env);
    closure = pop();

    if (isBuiltIn(closure))
        {
        //printf("call is a builtin\n");
        return evalBuiltIn(eargs,closure);
        }
    else
        {
        int body, xenv;
        //printf("call is user defined\n");
        
        assureMemory(OBJECT_CELLS+THUNK_CELLS,&closure,&params,&eargs,0);

        body = closure_body(closure);
        xenv = closure_context(closure);
        xenv = makeObject(xenv,closure,params,eargs);
        return makeThunk(body,xenv);
        }
    }

static int
evalBuiltIn(int args,int builtIn)
    {
    PRIM prim;

    //printf("builtIn arguments are: ");
    //pp(stdout,args);
    //printf("\n");

    //printf("closure body is: ");
    //pp(stdout,closure_body(builtIn));
    //printf("\n");

    prim = BuiltIns[ival(closure_body(builtIn))];
    return prim(args);
    }

/* evalListExceptLast expects a list of thunks */

int
evalListExceptLast(int items)
    {
    int result = 0;
    while (cdr(items) != 0)
        {
        push(items);
        result = eval(thunk_code(car(items)),thunk_context(car(items)));
        items = pop();

        items = cdr(items);
        }
    return car(items);
    }

/* evalList expects a list of thunks */

int
evalList(int items)
    {
    int result = 0;
    while (items != 0)
        {
        push(items);
        result = eval(thunk_code(car(items)),thunk_context(car(items)));
        items = pop();

        items = cdr(items);
        }
    return result;
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
    else if (sameSymbol(car(params),sharpSymbol))
        return cons(makeThunk(unevaluatedList(args),env),0);
    else if (sameSymbol(car(params),dollarSymbol))
        return cons(thunkizedList(args,env),0);
    else if (sameSymbol(car(params),atSymbol))
        return cons(evaluatedList(args,env),0);
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
thunkizedList(int args,int env)
    {
    if (args == 0)
        return 0;
    else
        return cons(makeThunk(car(args),env),thunkizedList(cdr(args),env));
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
