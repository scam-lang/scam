#include "types.h"
#include "cell.h"
#include "env.h"
#include "run.h"

int
evalR(int expr,int env)
    {
    run(analyze(expr),env);
    }

int
analyze(int expr)
    {
    if (type(expr) == INTEGER) return makeRun(runIdentity,expr);
    if (type(expr) == REAL) return makeRun(runIdentity,expr);
    if (type(expr) == STRING) return makeRun(runIdentity,expr);
    if (sameSymbol(expr,trueSymbol)) return makeRun(runIdentity,expr);
    if (sameSymbol(expr,falseSymbol)) return makeRun(runIdentity,expr);
    if (sameSymbol(expr,nilSymbol)) return makeRun(runIdentity,0);
    if (type(expr) == SYMBOL) return makeRun(runLookup,expr);
    if (type(car(expr)) == SYMBOL && ival(car(expr)) == ival(objectSymbol))
        return makeRun(runIdentity,expr);
    //must be a call
    return analyzeCall(expr);
    }

int
analyzeCall(int expr)
    {
    int args;

    args = analyzeArgs(expr);

    return makeRun(runCall,args);
    }

int
analyzeArgs(int expr)
    {
    int first,rest;

    if (expr == 0) return 0;

    push(expr);
    first = analyze(car(expr));
    expr = pop();
    push(first);
    rest = analyzeArgs(cdr(expr));
    first = pop();

    return cons(first,rest);
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

int
makeRun(int (*f)(int,int),int expr)
    {
    assureMemory("makeRun",3,&expr,(int *)0);
    int args = ucons(expr,0);
    int flexeme = ucons(0,0);
    fval(flexeme) = f;
    return ucons(args,flexeme);
    }

int
runIdentity(int args,int env)
    {
    return car(args);
    }

int
runLookup(int args,int env)
    {
    return getVariableValue(car(args),env);
    }

int
runArgs(int expr,int env)
    {
    int first,rest;

    if (expr == 0) return 0;

    push(expr);
    push(env);
    first = run(car(expr),env);
    env = pop();
    expr = pop();
    push(first);
    rest = runArgs(cdr(expr),env);
    first = pop();

    return cons(first,rest);
    }

int
runCall(int args,int env)
    {
    int args = runArgs(args,env);
    }

