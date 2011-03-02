#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "cell.h"
#include "types.h"
#include "env.h"
#include "pp.h"
#include "util.h"

int
getVariableValue(int var,int env)
    {
    int spot = findLocation(ival(var),env);
    if (spot == 0)
        {
        //ppObject(stdout,env,0);
        return throw("variable %s is undefined",
            SymbolTable[ival(var)]);
        }
    if (sameSymbol(car(spot),uninitializedSymbol))
        {
        return throw("variable %s is uninitialized",
            SymbolTable[ival(var)]);
        }
    return car(spot);
    }
        
int
setVariableValue(int var,int val,int env)
    {
    int spot = findLocation(ival(var),env);
    if (spot == 0)
        {
        return throw("variable %s is undefined",SymbolTable[ival(var)]);
        }
    car(spot) = val;
    return val;
    }

int 
defineVariable(int env,int var,int val)
    {
    int i;
    int vars,vals;

    //printf("defining variable %s\n",SymbolTable[ival(var)]);
    //debug("env",env);

    assert(DEFINE_CELLS == 2);

    assureMemory("defineVariable",DEFINE_CELLS,&env,&var,&val,0);

    /* there are predefined variables, skip over those */

    vars = cadr(env);
    vals = caddr(env);
    for (i = 0; i < ENV_PREDEFINED - 1; ++i)
        {
        vars = cdr(vars);
        vals = cdr(vals);
        }
    cdr(vars) = ucons(var,cdr(vars));
    cdr(vals) = ucons(val,cdr(vals));
 
    //debug("defined value",val);
    return val;
    }

int
makeObject(int type) /* not gc-safe, caller needs to ensure OBJECT_CELLS */
    {
    int o;
    int vars,vals;

    assert(OBJECT_CELLS == 5);

    vars = ucons(labelSymbol,0);
    vals = ucons(type,0);

    o = ucons(objectSymbol,ucons(vars,ucons(vals,0)));

    return o;
    }

int
makeEnvironment(int context,int constructor,int vars,int vals)
    {
    int o;

    assert(ENV_CELLS == OBJECT_CELLS + 6);

    assureMemory("makeEnvironment",ENV_CELLS,
        &context,&constructor,&vars,&vals,0);

    o = makeObject(envSymbol);

    object_variable_hook(o) =
        ucons(contextSymbol,
            ucons(constructorSymbol,
                ucons(thisSymbol,vars)));

    object_value_hook(o) =
        ucons(context,
            ucons(constructor,
                ucons(o,vals)));

    return o;
    }

int
makeThunk(int expr,int env)
    {
    int o;

    assert(THUNK_CELLS == OBJECT_CELLS + 4);

    assureMemory("makeThunk",THUNK_CELLS,&expr,&env,0);

    o = makeObject(thunkSymbol);

    object_variable_hook(o) =
        ucons(contextSymbol,
            ucons(codeSymbol,0));

    object_value_hook(o) =
        ucons(env,
            ucons(expr,0));

    return o;
    }

int
makeClosure(int context,int name,int parameters,int body,int mode)
    {
    int o;

    assert(CLOSURE_CELLS == OBJECT_CELLS + 8 + 1);

    assureMemory("makeClosure",CLOSURE_CELLS,
        &context,&name,&parameters,&body,0);

    o = makeObject(closureSymbol);

    if (mode == ADD_BEGIN)
       body = ucons(beginSymbol,body);

    object_variable_hook(o) =
        ucons(contextSymbol,
            ucons(nameSymbol,
                ucons(parametersSymbol,
                    ucons(codeSymbol,0))));

    object_value_hook(o) =
        ucons(context,
            ucons(name,
                ucons(parameters,
                    ucons(body,0))));

    return o;
    }

int
makeBuiltIn(int env,int name,int parameters,int body)
    {
    int b = makeClosure(env,name,parameters,body,NO_BEGIN);

    object_label(b) = builtInSymbol;

    return b;
    }

int
makeError(int tag,int expr,int msg,int trace)
    {
    int o;

    assert(ERROR_CELLS == OBJECT_CELLS + 6);

    assureMemory("makeError",ERROR_CELLS,
        &tag,&msg,&trace,0);

    o = makeObject(tag);

    object_variable_hook(o) =
        ucons(codeSymbol,
            ucons(messageSymbol,
                ucons(traceSymbol,0)));

    object_value_hook(o) =
        ucons(expr,
            ucons(msg,
                ucons(trace,0)));

    return o;
    }

int
convertThrow(int tag,int e)
    {
    object_label(e) = tag;

    return e;
    }

int
makeThrow(int expr,int msg,int trace)
    {
    return makeError(throwSymbol,expr,msg,trace);
    }

int
throw(char *fmt, ...)
    {
    va_list ap;
    int s;
    char buffer[512];

    //printf("encountered a fatal error...\n");

    va_start(ap, fmt);
    vsnprintf(buffer,sizeof(buffer), fmt, ap);
    va_end(ap);

    s = newString(buffer);

    return makeThrow(0,s,0);
    }

int
throwAgain(int expr,int exception)
    {
    //debug("adding trace expression",expr);
    //printf("adding trace file %s, line %d\n",
        //SymbolTable[file(expr)],line(expr));

    return makeThrow(expr,0,exception);
    }

int
findLocation(int index,int env)
    {
    while (env != 0)
        {
        int vars = object_variables(env);
        int vals = object_values(env);
        while (vars != 0)
            {
            //printf("looking at %s\n",SymbolTable[ival(car(vars))]);
            if (ival(car(vars)) == index) return vals;
            vars = cdr(vars);
            vals = cdr(vals);
            }
        env = env_context(env);
        //printf("not in this environment, how about...");
        //pp(stdout,env);
        }

   return 0;
   }
