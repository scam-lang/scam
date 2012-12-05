#include <stdio.h>
#include <assert.h>
#include "types.h"
#include "cell.h"
#include "env.h"
#include "prim.h"
#include "pp.h"
#include "run.h"

static int analyze(int);
static int analyzeCall(int);

static int runBuiltIn(int args,int builtIn);
static int runExprList(int items,int env);

static int unevaluatedArgList(int);
static int evaluatedArgList(int,int);
static int processArguments(int,int,int,int,int,int);

int
eval(int expr,int env)
    {
    return run(analyze(expr),env);
    }

static int
analyze(int expr)
    {
    if (type(expr) == INTEGER) return makeRunner(runIdentity,expr);
    if (type(expr) == REAL) return makeRunner(runIdentity,expr);
    if (type(expr) == STRING) return makeRunner(runIdentity,expr);
    if (sameSymbol(expr,trueSymbol)) return makeRunner(runIdentity,expr);
    if (sameSymbol(expr,falseSymbol)) return makeRunner(runIdentity,expr);
    if (sameSymbol(expr,nilSymbol)) return makeRunner(runIdentity,0);
    if (type(expr) == SYMBOL) return makeRunner(runLookup,expr);
    if (type(car(expr)) == SYMBOL && ival(car(expr)) == ival(objectSymbol))
        return makeRunner(runIdentity,expr);
    //must be a call
    return analyzeCall(expr);
    }

static int
analyzeCall(int args)
    {
    int cell,result,spot,arg;

    if (args == 0) return 0;

    /* analyze the first arg */

    push(args);
    arg = analyze(car(args));

    /* start the backbone */

    assureMemory("analyzeArgs",1,&arg,(int *)0);
    cell = ucons(arg,0);

    args = pop();
    args = cdr(args);

    result = cell;
    spot = cell;

    /* now analyze remaining args, adding to the backbone */

    while (args != 0)
        {
        push(args);
        arg = analyze(car(args));

        /* make new backbone cell */

        assureMemory("analyzeArgs",1,&arg,(int *)0);
        cell = ucons(arg,0);
        cdr(spot) = cell;
        spot = cell;

        args = pop();
        args = cdr(args);
        }

    //debug("analysis of call: ",result);
    return makeRunner(runCall,result);
    }

int
makeRunner(int (*f)(int,int),int expr)
    {
    int result;

    assert(RUNNER_CELLS == 2);
    assureMemory("makeRunner",2,&expr,(int *)0);
    int flexeme = ucons(0,0);
    fval(flexeme) = f;
    result = ucons(expr,flexeme);

    type(result) = RUNNER;
    file(result) = file(expr);
    line(result) = line(expr);

    return result;
    }

int
runIdentity(int expr,int env)
    {
    return expr;
    }

int
runLookup(int id,int env)
    {
    return getVariableValue(id,env);
    }

int
runCall(int call,int env)
    {
    int closure,eargs;
    int callingLevel = ival(env_level(env));

    /* evaluate the operator, getting the closure */

    //debug("runCall, call is ",call);

    /* the one argument to call is the list of arguments, so extract the list */

    //debug("op is ",car(call));

    push(call);
    push(env);
    closure = run(car(call),env);
    env = pop();
    call = pop();

    //debug("closure is ",closure);

    rethrow(closure,0);

    /* at this point, need to check for implicit and delayed args */

    if (isClosure(closure))
        {
        int result;
        //printf("it's a closure!\n");
        push(closure);
        //debug("unevaluated args",cdr(call));
        eargs = processArguments(closure_name(closure),
            closure_parameters(closure),cdr(call),env,
            file(car(call)),line(car(call)));
        closure = pop();

        rethrow(eargs,0);

        //debug("evaluated args",eargs);

        if (sameSymbol(object_label(closure),builtInSymbol))
            {
            result = runBuiltIn(eargs,closure);
            //debug("builtin call result",result);
            }
        else
            {
            int params,body,xenv;
            
            assureMemory("evalCall",OBJECT_CELLS+THUNK_CELLS + 1,&closure,&eargs,(int *)0);

            params = closure_parameters(closure);
            body = closure_body(closure);
            xenv = closure_context(closure);
            xenv = makeEnvironment(xenv,closure,params,eargs);

            //debug("body: ",body);

            env_level(xenv) = newInteger(callingLevel + 1);

            //debug("calling",car(call));
            result = makeThunk(body,xenv);
            }
        return result;
        }
    else if (isObject(closure))
        {
        //printf("it's an object!\n");
        //debug("unevaluated args",cdr(call));
        push(closure);
        eargs = runExprList(cdr(call),env);
        closure = pop();

        rethrow(eargs,0);

        //debug("evaluated args",eargs);
        //printf("starting to walk the object\n");
        while (eargs != 0)
            {
            if (!isObject(closure))
                {
                return throw(nonObjectSymbol,
                    "file %s,line %d: "
                    "attempted to access %s as an object",
                    SymbolTable[file(closure)],line(closure),
                    type(closure));
                }
            closure = getVariableValue(car(eargs),closure);
            //debug("object now is",closure);
            rethrow(closure,0);
            eargs = cdr(eargs);
            }

        return closure;
        }
    else
        {
        return throw(nonFunctionSymbol,
            "file %s,line %d: "
            "attempted to call %s as a function",
            SymbolTable[file(closure)],line(closure),
            type(closure));
        }

    return 0;
    }

int
run(int runner,int env)
    {
    int result;
    int returns = 0;
    int level = ival(env_level(env));
    int (*f)(int,int);

    while (1)
        {
        //debug("run: ",runner);
        assert(type(runner) == RUNNER);

        f = fval(cdr(runner));
        result = f(car(runner),env);

        if (isReturn(result))
            {
            int s = error_value(result);
            if (level < (ival(env_level(thunk_context(s))) - returns))
                {
                result = s;
				returns++;
                }
			else
				{
				ival(env_level(thunk_context(s))) = ival(env_level(thunk_context(s))) - returns; 
				}
            }
        else if (isThrow(result))
            {
            /*
            if (!(type((car(runner)) == CONS) || !(sameSymbol(car(expr),beginSymbol)))
                {
                int et;
                push(result);
                et = cons(expr,error_trace(result));
                result = pop();
                error_trace(result) = et;
                }
            */
            break;
            }

        if (!isThunk(result)) break;

        runner = thunk_code(result);
        env = thunk_context(result);
		
		// Keep the level at lowest setting to properly handle returns
		level = ival(env_level(env))<level?ival(env_level(env)):level;
        }

    //debug("final result",result);
    //debug("original expression was",orig);

    return result;
    }

static int
processArguments(int name,int params,int args,int env,int fi,int li)
    {
    int first,rest,result;

    //debug("p-a",params);
    if (params == 0 && args == 0)
        return 0;

    if (params == 0)
        {
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "too many arguments to function %s",
            SymbolTable[fi],li,
            SymbolTable[ival(name)]);
        }

    if (sameSymbol(car(params),atSymbol))
        {
        rest = evaluatedArgList(args,env);
        rethrow(rest,0);
        assureMemory("processArgs:eArgs",1,&rest,(int *)0);
        result = uconsfl(rest,0,fi,li);
        }
    else if (sameSymbol(car(params),dollarSymbol))
        {
        assureMemory("processArgs:amp",1 + length(args),&args,(int *)0);
        rest = unevaluatedArgList(args);
        result = uconsfl(rest,0,fi,li);
        }
    else if (sameSymbol(car(params),sharpSymbol))
        {
        push(env);
        rest = processArguments(name,cdr(params),args,env,fi,li);
        env = pop();

        rethrow(rest,0);

        assureMemory("processArgs:sharp",1,&env,&rest,(int *)0);
        result = uconsfl(env,rest,fi,li);
        }
    else if (args == 0)
        {
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "too few arguments to function %s",
            SymbolTable[fi],li,
            SymbolTable[ival(name)]);
        }
    else if (*SymbolTable[ival(car(params))] == '$')
        {
        push(args);
        rest = processArguments(name,cdr(params),cdr(args),env,fi,li);
        assureMemory("processArgs:tArg",1,&rest,(int *)0);
        args = pop();

        rethrow(rest,0);

        result = uconsfl(car(args),rest,fi,li);
        }
    else
        {
        push(env);
        push(args);
        push(name);
        push(params);
        first = run(car(args),env);
        params = pop();
        name = pop();
        args = pop();
        env = pop();

        rethrow(first,0);

        push(first);
        rest = processArguments(name,cdr(params),cdr(args),env,fi,li);
        assureMemory("processArgs:eArg",1,&rest,(int *)0);
        first = pop();

        rethrow(rest,0);

        result = uconsfl(first,rest,fi,li);
        }
    //debug("p-a result",result);
    return result;
    }

/* 
 * the caller of unevaluatedList is responsible for ensuring that
 * there are length(args) cells available
 */

static int
unevaluatedArgList(int args)
    {
    if (args == 0)
        return 0;
    else
        return ucons(car(args),unevaluatedArgList(cdr(args)));
    }

static int
evaluatedArgList(int args,int env)
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
        first = run(car(args),env);
        args = pop();
        env = pop();

        rethrow(first,0);

        //debug("evaluatedArgList: args",args);
        //debug("evaluatedArgList: env",env);

        push(first);
        rest = evaluatedArgList(cdr(args),env);
        assureMemory("evaluatedArgList",1,&rest,(int *)0);
        //printf("back from eval\n");
        first = pop();

        rethrow(rest,0);

        return ucons(first,rest);
        }
    }

/* runExprList expects a regular list of expressions */

static int
runExprList(int items,int env)
    {
    int result = 0;
    int spot = 0;

    if (items == 0) return 0;

    push(env);
    push(items);
    result = run(car(items),env);
    result = cons(result, 0);
    items = pop();
    env = pop();

    push(result);

    spot = result;
    items = cdr(items);

    while (items != 0)
        {
        int value;
        //debug("items before",items);
        push(env);
        push(items);
        push(spot);
        value = run(car(items),env);
        spot = pop();
        items = pop();
        env = pop();
        //debug("items after",items);

        rethrow(value,0);

        assureMemory("runExprList",1,
            &spot,&env,&items,&value,(int *) 0);

        cdr(spot) = ucons(value,0);
        spot = cdr(spot);

        items = cdr(items);
        }

    result = pop();
    return result;
    }

int
runList(int items,int env,int mode)
    {
    int result = 0;
    while (cdr(items) != 0)
        {
        //debug("items before",items);
        push(env);
        push(items);
        result = run(car(items),env);
        items = pop();
        env = pop();
        //debug("items after",items);

        if (isThrow(result))
            {
            return result;
            }

        items = cdr(items);
        }

    if (mode == ALLBUTLAST)
        {
        assureMemory("evalListExceptLast",THUNK_CELLS,&env,&items,(int *)0);
        return makeThunk(car(items),env);
        }
    else
        return run(car(items),env);
    }

static int
runBuiltIn(int args,int builtIn)
    {
    PRIM prim;

    //printf("builtIn arguments are: ");
    //pp(stdout,args);
    //printf("\n");

    prim = BuiltIns[ival(closure_body(builtIn))];
    return prim(args);
    }

int
extractRunner(int runner)
    {
    int expr = car(runner);
    if (type(expr) == SYMBOL) return expr;
    return extractRunnerList(expr);
    }

int
extractRunnerList(int items)
    {
    int cell,result,spot;

    debug("extractRunnerList: ",items);

    if (items == 0) return 0;

    assureMemory("analyzeArgs",length(items),&items,(int *)0);

    /* start the backbone */

    assert(type(car(items)) == RUNNER);

    cell = ucons(caar(items),0);

    items = cdr(items);

    result = cell;
    spot = cell;

    /* now copy remaining items, adding to the backbone */

    while (items != 0)
        {
        assert(type(car(items)) == RUNNER);

        /* make new backbone cell */

        cell = ucons(caar(items),0);
        cdr(spot) = cell;
        spot = cell;

        items = cdr(items);
        }

    return result;
    }

