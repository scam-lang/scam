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
static int evaluatedArgList(int,int);
static int unevaluatedArgList(int);

int
eval(int expr, int env)
    {
    int result;
    int level = ival(env_level(env));

    //debug("initial eval",expr);
    //printf("env level is %d\n",level);

    while (1)
        {
        int tag;

        //debug("eval",expr);
        //printf("env level is %d\n",ival(env_level(env)));
        //debug(" env",env);

        if (expr == 0) { result = 0; break; }
        if (type(expr) == INTEGER) { result = expr; break; }
        if (type(expr) == REAL)    { result = expr; break; }
        if (type(expr) == STRING)  { result = expr; break; }
        if (type(expr) == SYMBOL)
            {
            int index = ival(expr);
            if (index == ival(trueSymbol))
                result = trueSymbol;
            else if (index == ival(falseSymbol))
                result = falseSymbol;
            else if (index == ival(nilSymbol))
                result = 0;
            else
                result = getVariableValue(expr,env);
            break;
            }

        //printf("eval type is %s\n",type(expr));
        assert(type(expr) == CONS);

        tag  = car(expr);

        if (type(tag) == SYMBOL && ival(tag) == ival(objectSymbol))
            {
            result = expr;
            break;
            }

        /* no need to assure memory here */

        push(env);
        result = evalCall(expr,env,NORMAL);
        env = pop();

        if (isReturn(result))
            {
            int s = error_value(result);
            //printf("it's a return!\n");
            //debug("return expression",thunk_code(s));
            //printf("return level: %d\n",ival(env_level(thunk_context(s))));
            //printf("current level: %d\n",ival(env_level(env)));
            if (level < ival(env_level(thunk_context(s))))
                {
                result = s;
                //debug("result is now",result);
                }
            }

        if (isThrow(result)) break;

        if (!isThunk(result)) break;

        expr = thunk_code(result);
        env = thunk_context(result);
        }

    //debug("final result",result);
    return result;
    }
        
int
evalCall(int call,int env, int mode)
    {
    int closure,eargs;

    if (mode == NORMAL)
        {
        push(env);
        push(call);
        closure = eval(car(call),env);
        call = pop();
        env = pop();

        rethrow(closure,0);
        }
    else
        closure = car(call);
    //printf("done getting closure\n");

    //debug("evalCall",call);
    //debug("calling",closure);

    if (!isClosure(closure))
        return throw(nonFunctionSymbol,
            "attempted to call %s as a function", type(closure));

    /* args are the cdr of call */

    push(closure);
    //debug("unevaluated args",cdr(call));
    eargs = processArguments(closure_name(closure),
        closure_parameters(closure),cdr(call),env,mode);
    closure = pop();

    rethrow(eargs,0);

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
        
        assureMemory("evalCall",OBJECT_CELLS+THUNK_CELLS + 1,&closure,&eargs,0);

        params = closure_parameters(closure);
        body = closure_body(closure);
        xenv = closure_context(closure);
        xenv = makeEnvironment(xenv,closure,params,eargs);
        env_level(xenv) = newInteger(ival(env_level(env)) + 1);

        //debug("calling",car(call));
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

/* evalList expects a regular list of expressions */

int
evalList(int items,int env,int mode)
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
            {
            //if (isReturn(result) && ival(env_level(env) < thunk_context(error_value(result)))
            //    return error_value(result);
            //else
            //    return result;
            return result;
            }
        //if (isThrow(result))
        //    return throwAgain(car(items),result);

        items = cdr(items);
        }

    if (mode == ALLBUTLAST)
        {
        assureMemory("evalListExceptLast",THUNK_CELLS,&env,&items,0);
        return makeThunk(car(items),env);
        }
    else
        return eval(car(items),env);
    }

static int
processArguments(int name, int params,int args,int env,int mode)
    {
    int first,rest,result;

    //debug("p-a",params);
    if (params == 0 && args == 0)
        result = 0;
    else if (params == 0)
        {
        return throw(exceptionSymbol,"too many arguments to function %s",
            SymbolTable[ival(name)]);
        }
    else if (sameSymbol(car(params),atSymbol))
        {
        if (mode == NORMAL)
            {
            rest = evaluatedArgList(args,env);
            rethrow(rest,0);
            }
        else
            rest = unevaluatedArgList(args);
        assureMemory("processArgs:eArgs",1,&rest,0);
        result = ucons(rest,0);
        }
    else if (sameSymbol(car(params),dollarSymbol))
        {
        assureMemory("processArgs:amp",1 + length(args),&args,0);
        rest = unevaluatedArgList(args);
        result = ucons(rest,0);
        }
    else if (sameSymbol(car(params),sharpSymbol))
        {
        push(env);
        rest = processArguments(name,cdr(params),args,env,mode);
        env = pop();

        rethrow(rest,0);

        assureMemory("processArgs:sharp",1,&env,&rest,0);
        result = ucons(env,rest);
        }
    else if (args == 0)
        {
        return throw(exceptionSymbol,"too few arguments to function %s",
            SymbolTable[ival(name)]);
        }
    else if (*SymbolTable[ival(car(params))] == '$')
        {
        push(args);
        rest = processArguments(name,cdr(params),cdr(args),env,mode);
        assureMemory("processArgs:tArg",1,&rest,0);
        args = pop();

        rethrow(rest,0);

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

            rethrow(first,0);
            }
        else
            first = car(args);

        push(first);
        rest = processArguments(name,cdr(params),cdr(args),env,mode);
        assureMemory("processArgs:eArg",1,&rest,0);
        first = pop();

        rethrow(rest,0);

        result = ucons(first,rest);
        }
    //debug("p-a result",result);
    return result;
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

        rethrow(first,0);

        //debug("evaluatedArgList: args",args);
        //debug("evaluatedArgList: env",env);

        push(first);
        rest = evaluatedArgList(cdr(args),env);
        assureMemory("evaluatedArgList",1,&rest,0);
        //printf("back from eval\n");
        first = pop();

        rethrow(rest,0);

        return ucons(first,rest);
        }
    }
