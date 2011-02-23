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

static int evalBuiltIn(int,int);
static int processArguments(int,int,int,int,int);
static int thunkizedArgList(int,int);
static int evaluatedArgList(int,int);
static int unevaluatedArgList(int);

int
eval(int expr, int env)
    {
    while (1)
        {
        int t,tag;

        //debug("eval",expr);

        if (expr == 0) return 0;
        if (type(expr) == INTEGER) return expr;
        if (type(expr) == REAL)    return expr;
        if (type(expr) == STRING)  return expr;
        if (type(expr) == SYMBOL)
            {
            int index = ival(expr);
            if (index == ival(trueSymbol))
                return trueSymbol;
            else if (index == ival(falseSymbol))
                return falseSymbol;
            else if (index == ival(nilSymbol))
                return 0;
            else
                return getVariableValue(expr,env);
            }

        //printf("eval type is %s\n",type(expr));
        assert(type(expr) == CONS);

        tag  = car(expr);

        if (type(tag) == SYMBOL && ival(tag) == ival(objectSymbol))
            return expr;

        /* no need to assure memory here */

        t = evalCall(expr,env,NORMAL);

        if (!isThunk(t)) return t;

        expr = thunk_code(t);
        env = thunk_context(t);
        }

    return 0;
    }
        
int
evalCall(int call,int env, int mode)
    {
    int closure,eargs;

    //printf("getting closure\n");
    if (mode == NORMAL)
        {
        push(env);
        push(call);
        closure = eval(car(call),env);
        call = pop();
        env = pop();

        rethrow(closure);
        }
    else
        closure = car(call);
    //printf("done getting closure\n");

    //debug("evalCall",call);
    //debug("calling",closure);

    if (!isClosure(closure))
        return throw("attempted to call %s as a function", type(closure));

    /* args are the cdr of call */

    push(closure);
    //debug("unevaluated args",cdr(call));
    eargs = processArguments(closure_name(closure),
        closure_parameters(closure),cdr(call),env,mode);
    closure = pop();

    rethrow(eargs);

    //debug("evaluated args",eargs);

    if (sameSymbol(object_label(closure),builtInSymbol))
        {
        int result;
        result = evalBuiltIn(eargs,closure);
        //debug("builtin call result",result);
        return result;
        }
    else
        {
        int params,body,xenv;
        
        assureMemory("evalCall",OBJECT_CELLS+THUNK_CELLS,&closure,&eargs,0);

        params = closure_parameters(closure);
        body = closure_body(closure);
        xenv = closure_context(closure);
        xenv = makeEnvironment(xenv,closure,params,eargs);
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

/* evalThunkListExceptLast expects a list of thunks */

int
evalThunkListExceptLast(int items)
    {
    while (cdr(items) != 0)
        {
        int result;
        //debug("items before",items);
        push(items);
        result = eval(thunk_code(car(items)),thunk_context(car(items)));
        items = pop();
        //debug("items after",items);

        if (isThrow(result))
            return throwAgain(file(car(items)),line(car(items)),result);

        items = cdr(items);
        }
    return car(items);
    }

/* evalListExceptLast expects a regular list of expressions */

int
evalListExceptLast(int items,int env)
    {
    int result = 0;
    while (cdr(items) != 0)
        {
        //debug("items before",items);
        push(env);
        push(items);
        result = eval(car(items),env);
        items = pop();
        env = pop();
        //debug("items after",items);

        if (isThrow(result))
            return throwAgain(file(car(items)),line(car(items)),result);

        items = cdr(items);
        }
    assureMemory("evalListExceptLast",THUNK_CELLS,&env,&items,0);
    return makeThunk(car(items),env);
    }

/* evalThunkList expects a list of thunks */

int
evalThunkList(int items)
    {
    int result;
    while (items != 0)
        {
        //debug("items before",items);
        push(items);
        result = eval(thunk_code(car(items)),thunk_context(car(items)));
        items = pop();
        //debug("items after",items);

        if (isThrow(result))
            return throwAgain(file(car(items)),line(car(items)),result);

        items = cdr(items);
        }
    return result;
    }
    


static int
processArguments(int name, int params,int args,int env,int mode)
    {
    int ch,first,rest,result;

    //debug("p-a",params);
    if (params == 0 && args == 0)
        result = 0;
    else if (params == 0)
        {
        return throw("too many arguments to function %s",
            SymbolTable[ival(name)]);
        }
    else if (sameSymbol(car(params),sharpSymbol))
        {
        assureMemory("processArgs:uArgs",THUNK_CELLS + 1 + length(args),&env,&args,0);
        rest = unevaluatedArgList(args);
        result = ucons(makeThunk(rest,env),0);
        }
    else if (sameSymbol(car(params),dollarSymbol))
        {
        rest = thunkizedArgList(args,env);
        assureMemory("processArgs:tArgs",1,&rest,0);
        result = ucons(rest,0);
        }
    else if (sameSymbol(car(params),atSymbol))
        {
        if (mode == NORMAL)
            {
            rest = evaluatedArgList(args,env);
            rethrow(rest);
            }
        else
            rest = unevaluatedArgList(args);
        assureMemory("processArgs:eArgs",1,&rest,0);
        result = ucons(rest,0);
        }
    else if (sameSymbol(car(params),ampersandSymbol))
        {
        assureMemory("processArgs:amp",1 + length(args),&args,0);
        rest = unevaluatedArgList(args);
        result = ucons(rest,0);
        }
    else if (sameSymbol(car(params),hatSymbol))
        {
        push(env);
        rest = processArguments(name,cdr(params),args,env,mode);
        env = pop();

        rethrow(rest);

        assureMemory("processArgs:hat",1,&env,&rest,0);
        result = ucons(env,rest);
        }
    else if (args == 0)
        {
        return throw("too few arguments to function %s",
            SymbolTable[ival(name)]);
        }
    else if ((ch = *SymbolTable[ival(car(params))]) == '$' || ch == '#')
        {
        push(env);
        push(args);
        rest = processArguments(name,cdr(params),cdr(args),env,mode);
        assureMemory("processArgs:tArg",THUNK_CELLS + 1,&rest,0);
        args = pop();
        env = pop();

        rethrow(rest);

        result = ucons(makeThunk(car(args),env),rest);
        }
    else if (ch == '&')
        {
        push(args);
        rest = processArguments(name,cdr(params),cdr(args),env,mode);
        assureMemory("processArgs:tArg",1,&rest,0);
        args = pop();

        rethrow(rest);

        result = ucons(car(args),rest);
        }
    else
        {
        if (mode == NORMAL)
            {
            push(env);
            push(args);
            push(name);
            push(params);
            first = eval(car(args),env);
            params = pop();
            name = pop();
            args = pop();
            env = pop();

            rethrow(first);
            }
        else
            first = car(args);

        push(first);
        rest = processArguments(name,cdr(params),cdr(args),env,mode);
        assureMemory("processArgs:eArg",1,&rest,0);
        first = pop();

        rethrow(rest);

        result = ucons(first,rest);
        }
    //debug("p-a result",result);
    return result;
    }

static int
thunkizedArgList(int args,int env)
    {
    int rest;
    if (args == 0)
        return 0;
    else
        {
        push(env);
        push(args);
        rest = thunkizedArgList(cdr(args),env);
        assureMemory("thunkizedArgList",THUNK_CELLS + 1,&rest,0);
        args = pop();
        env = pop();

        return ucons(makeThunk(car(args),env),rest);
        }
    }

/* 
 * the caller of unevaluatedList is responsible for ensuring that
 * there are length(args) cells available
 */

static int
unevaluatedArgList(args)
    {
    if (args == 0)
        return 0;
    else
        return ucons(car(args),unevaluatedArgList(cdr(args)));

    }

static int
evaluatedArgList(args,env)
    {
    int first;
    int rest;
    if (args == 0)
        return 0;
    else
        {
        //debug("need to evaluate arg",car(args));
        push(env);
        push(args);
        first = eval(car(args),env);
        args = pop();
        env = pop();

        rethrow(first);

        //debug("evaluatedArgList: args",args);
        //debug("evaluatedArgList: env",env);

        push(first);
        rest = evaluatedArgList(cdr(args),env);
        assureMemory("evaluatedArgList",1,&rest,0);
        //printf("back from eval\n");
        first = pop();

        rethrow(rest);

        return ucons(first,rest);
        }
    }
