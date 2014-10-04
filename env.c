
/*
 *  Main Author : John C. Lusth
 *  Barely Authors : Jeffrey Robinson, Gabriel Loewen
 *  Last Modified : May 4, 2014
 *
 *  TODO : Description
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/time.h>

#include "scam.h"
#include "cell.h"
#include "types.h"
#include "env.h"
#include "pp.h"
#include "util.h"

/*
 * getVariableValue - look up a variable in an environment
 *
 *  Not heap safe
 */

int
getVariableValue(int var,int env)
    {
    /* findLocation returns the variable's spot */
    int spot = findLocation(ival(var),env);
    if (spot == 0)
        {
        validate("getVariable");
        //printf("current envronment chain:\n");
        //while (env != 0)
        //    {
        //    ppTable(stdout,env,0);
        //    printf("------\n");
        //    env = env_context(env);
        //    }
        return throw(UndefinedVariableSymbol,
            "file %s,line %d: "
            "variable %s is undefined",
            SymbolTable[file(var)],line(var),
            SymbolTable[ival(var)]);
        }
    int o;
    o = car(spot);
    spot = o;
    if (SameSymbol(spot,UninitializedSymbol))
        {
        return throw(
                UninitializedVariableSymbol,
                "file %s,line %d: "
                "variable %s is uninitialized",
                SymbolTable[file(var)],
                line(var),
                SymbolTable[ival(var)]
            );
        }
    return spot;
    }
        
/*
 * setVariableValue - update a variable in an environment
 *
 *  Not heap safe
 */

int
setVariableValue(int var,int val,int env)
    {
    /* findLocation returns the variable's spot */
    int spot = findLocation(ival(var),env);
    if (spot == 0)
        {
        return throw(
                ExceptionSymbol,
                "file %s,line %d: "
                "Variable %s is undefined",
                SymbolTable[file(var)],
                line(var),
                SymbolTable[ival(var)]
            );
        }
    setcar(spot,val);
    return val;
    }

/*
 * defineVariable - add a variable to an environment
 *
 */

int 
defineVariable(int env,int var,int val)
    {
    int vars,vals;

    /* need two cons cells */

    assert(DEFINE_VARIABLE_SIZE == 2);

    vars = env_variable_hook(env);
    vals = env_value_hook(env);
    
    /* caller is responsible for ensuring adequate memory */

    int a = cons(var,cdr(vars));
    int b = cons(val,cdr(vals));

    setcdr(vars,a);
    setcdr(vals,b);

    return val;
    }

/*
 * makeObject - generic object making routine
 *            - scam objects have the structure (object (label ...) (type ...))
 *
 */

int
makeObject(int type)
    {
    int vars,vals;

    assert(MAKE_OBJECT_SIZE == 5);

    /* caller is responsible for ensuring adequate memory */

    vals = cons(type,0);
    vars = cons(LabelSymbol,0);
    
    return cons(ObjectSymbol,cons(vars,cons(vals,0)));
    }

/*
 * makeEnvironment - generic environment making routine
 *                 - scam environments are rendered as objects and have
 *                   the structure:
 *   (object (label context level constructor this)
 *           (environment context level constructor object))
 *
 *  There are five predefined variables - if this changes, change the
 *  macros env_variable_hook and env_value_hook
 */

int
makeEnvironment(int context,int constructor,int vars,int vals)
    {
    int o;

    assert(MAKE_ENVIRONMENT_SIZE == 9 + MAKE_OBJECT_SIZE);

    /* caller is responsible for ensuring adequate memory */

    o = makeObject(EnvSymbol);

    /* Four cons */
    set_object_variable_hook(o,
        cons(ContextSymbol,
            cons(LevelSymbol,
                cons(ConstructorSymbol,
                    cons(ThisSymbol,vars)))));

    /* 4 cons and a possible integer */
    set_object_value_hook(o,
        cons(context,
            cons(context == 0? integerZero : env_level(context),
                cons(constructor,
                    cons(o,vals)))));

    return o;
    }

/*
 * makeThunk - generic thunk making routine
 *           - scam thunks are rendered as objects and have
 *             the structure:
 *   (object (label context code)
 *           (thunk context expr))
 *
 * Thunks are used to implement tail-recursion optimization.
 */

int
makeThunk(int expr,int env)
    {
    int o;

    /* caller is responsible for ensuring adequate memory */

    assert(MAKE_THUNK_SIZE == 4 + MAKE_OBJECT_SIZE);

    o = makeObject(ThunkSymbol);


    set_object_variable_hook(o,
        cons(ContextSymbol,
            cons(CodeSymbol,0)));

    set_object_value_hook(o,
        cons(env,
            cons(expr,0)));

    setfile(o,file(expr));
    setline(o,line(expr));
    return o;
    }

/*
 * makeClosure - generic closure making routine
 *             - scam closures are rendered as objects and have
 *               the structure:
 *   (object (label context name parameters code)
 *           (closure context name parameters body))
 *
 * Closures are used to implement function objects.
 */

int
makeClosure(int context,int name,int parameters,int body,int mode)
    {
    int o;

    assert(MAKE_CLOSURE_SIZE == 9 + MAKE_OBJECT_SIZE);

    /* caller is responsible for ensuring adequate memory */

    o = makeObject(ClosureSymbol);

    /* add BEGIN for user-defined only (not built-ins) */

    if (mode == ADD_BEGIN)
       {
       body = cons2(BeginSymbol,body);
       }

    set_object_variable_hook(o,
        cons(ContextSymbol,
            cons(NameSymbol,
                cons(ParametersSymbol,
                    cons(CodeSymbol,0)))));

    set_object_value_hook(o,
        cons(context,
            cons(name,
                cons(parameters,
                    cons(body,0)))));

    setfile(o,file(body));
    setline(o,line(body));
    return o;
    }

/*
 * makeBuiltIn - generic built-in function making routine
 *             - scam built-in functions are rendered as closures, but the
 *               the object label is changed to builtIn.
 */

int
makeBuiltIn(int env,int name,int parameters,int body)
    {
    int b;

    assert(MAKE_BUILTIN_SIZE == MAKE_CLOSURE_SIZE);

    /* caller is responsible for ensuring adequate memory */

    b = makeClosure(env,name,parameters,body,NO_BEGIN);

    set_object_label(b,BuiltInSymbol);

    return b;
    }

/*
 * makeThrow - make an exception object
 *           - scam exceptions are rendered as objects and have
 *               the structure:
 *   (object (label code value trace)
 *           (throw code value trace))
 *
 * Error objects arise when exceptions are caught.
 */

int
makeThrow(int code,int value,int trace)
    {
    int o;
    assert(MAKE_THROW_SIZE == 6 + MAKE_OBJECT_SIZE);

    /* caller is responsible for ensuring adequate memory */

    o = makeObject(ThrowSymbol);

    set_object_variable_hook(o,
        cons(CodeSymbol,
            cons(ValueSymbol,
                cons(TraceSymbol,0))));

    set_object_value_hook(o,
        cons(code,
            cons(value,
                cons(trace,0))));

    setfile(o,file(code));
    setline(o,line(code));
    return o;
    }

/*
 * throw - generate an exception from a format string
 *       - this is the routine that is called when an error is detected
 *         and you wish to throw an exception in Scam
 */

int
throw(int symbol,char *fmt, ...)
    {
    va_list ap;
    int s,result;
    char buffer[512];

    va_start(ap, fmt);
    vsnprintf(buffer,sizeof(buffer), fmt, ap);
    va_end(ap);

    PUSH(symbol);
    s = newString(buffer);

    P();
    ENSURE_MEMORY(MAKE_THROW_SIZE,&s,(int *) 0);
    symbol = POP(); /* OK to pop after ensureMemory */

    result = makeThrow(symbol,s,0);
    V();

    return result;
    }

/*
 * findLocation - helper function for get- and setVariableValue
 *              - returns the spot in the environment that holds the variable's
 *                value
 */

int
findLocation(int index,int env)
    {
    int outer = 0;
    while (env != 0)
        {
        ++outer;
        if(outer==1000)
            {
            printf("outer : %d\n" , outer);
            }
        int vars;
        vars = object_variables(env);

        int vals;
        vals = object_values(env);

        int inner=0;
        while (vars != 0)
            {
            ++inner;
            if (inner % 1000 == 0) printf("inner is %d\n",inner);
            if (ival(car(vars)) == index)
                {
                return vals;
                }
            vars = cdr(vars);
            vals = cdr(vals);
            }

        env = env_context(env);
        }

    return 0;
    }

/*
 * isLocal - returns true if the variable is defined in the local scope
 *
 */

int
findLocal(int var,int env)
    {
    int vars = object_variables(env);
    int vals = object_values(env);
    while (vars != 0)
        {
        if (ival(car(vars)) == ival(var)) return vals;
        vars = cdr(vars);
        vals = cdr(vals);
        }

    return 0;
    }
