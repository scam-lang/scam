#include <stdio.h>
#include <assert.h>
#include "cell.h"
#include "types.h"
#include "env.h"
#include "pp.h"
#include "util.h"

int
lookupVariableValue(int var,int env)
    {
    int spot = findLocation(ival(var),env);
    if (spot == 0)
        {
        Fatal("undefinedVariable"
            ": variable %s is undefined",
            SymbolTable[ival(var)]);
        }
    if (sameSymbol(car(spot),uninitializedSymbol))
        {
        Fatal("uninitializedVariable"
            ": variable %s is uninitialized",
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
        Fatal("undefinedVariable"
            ": variable %s is undefined",SymbolTable[ival(var)]);
        }
    car(spot) = val;
    return val;
    }

int 
defineVariable(int env,int var,int val)
    {
    int i;
    int vars,vals;

    //printf("defining variable %s\n",symbols[ival(var)]);
    //ppf("env: ",env,"\n");

    assert(DEFINE_CELLS == 2);

    assureMemory(2,val,var,env,0);

    /* there are five predefined variables, skip over those */

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
makeThunk(int expr,int env)
    {
    int vars,vals;
    
    assert(THUNK_CELLS == 7);

    assureMemory(7,expr,env,0);

    vars = ucons(contextSymbol,ucons(codeSymbol,0));
    vals = ucons(env,ucons(expr,0));


    return ucons(thunkSymbol,ucons(vars,ucons(vals,0)));
    }

int
makeBuiltIn(int env,int name,int parameters,int body)
    {
    int b = makeClosure(env,name,parameters,body,NO_BEGIN);

    car(b) = builtInSymbol;

    return b;
    }

int
makeObject(int context,int constructor,int vars,int vals)
    {
    int o;

    assert(OBJECT_CELLS == 9);

    assureMemory(9,context,constructor,vars,vals,0);

    vars = 
        ucons(contextSymbol,
            ucons(constructorSymbol,
                ucons(thisSymbol,vars)));
    vals =
        ucons(context,
            ucons(constructor,
                ucons(0,vals)));

    o = ucons(objectSymbol,ucons(vars,ucons(vals,0)));
    object_this(o) = o;

    return o;
    }

int
makeClosure(int context,int name,int parameters,int body,int mode)
    {
    int vars,vals;

    assert(CLOSURE_CELLS == 12);

    assureMemory(12,context,name,parameters,body,0);

    if (mode == ADD_BEGIN)
       body = ucons(beginSymbol,body);

    vars = 
        ucons(contextSymbol,
            ucons(nameSymbol,
                ucons(parametersSymbol,
                    ucons(codeSymbol,0))));
    vals =
        ucons(context,
            ucons(name,
                ucons(parameters,
                    ucons(body,0))));

    return ucons(closureSymbol,ucons(vars,ucons(vals,0)));
    }

int
makeError(int tag,int context,int expr,int kind,int value,int trace)
    {
    int vars,vals;

    assert(ERROR_CELLS == 13);

    assureMemory(13,tag,context,expr,kind,value,trace,0);

    vars =
        ucons(contextSymbol,
            ucons(codeSymbol,
                ucons(typeSymbol,
                    ucons(valueSymbol,
                        ucons(traceSymbol,0)))));
    vals =
        ucons(context,
            ucons(expr,
                ucons(kind,
                    ucons(value,
                        ucons(trace, 0)))));

    return ucons(tag,ucons(vars,ucons(vals,0)));
    }

int
makeThrow(int sym,int val,int env)
    {
    return makeError(throwSymbol,env,0,sym,val,0);
    }

int
makeErrorFromError(int tag,int e)
    {
    int m = makeError(tag,error_context(e),error_code(e),
                error_type(e),error_value(e),error_trace(e));

    line(m) = line(e);
    file(m) = file(e);

    return m;
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
        env = object_context(env);
        //printf("not in this environment, how about...");
        //pp(stdout,env);
        }

   return 0;
   }
