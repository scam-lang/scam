#include <stdio.h>
#include <assert.h>
#include "cell.h"
#include "types.h"
#include "env.h"
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
defineVariable(int var,int val,int env)
   {
   int i;
   int vars,vals;

   //printf("defining variable %s\n",symbols[ival(var)]);
   //ppf("env: ",env,"\n");

   /* there are five predefined variables, skip over those */

   vars = cadr(env);
   vals = caddr(env);
   for (i = 0; i < ENV_PREDEFINED - 1; ++i)
       {
       vars = cdr(vars);
       vals = cdr(vals);
       }
   cdr(vars) = cons(var,cdr(vars));
   cdr(vals) = cons(val,cdr(vals));

   return val;
   }

int
makeThunk(int expr,int env)
    {
    int vars,vals;
    
    vars = cons(contextSymbol,cons(codeSymbol,0));
    vals = cons(env,cons(expr,0));

    return cons(thunkSymbol,cons(vars,cons(vals,0)));
    }

int
makeClosure(int env,int rest)
    {
    int vars,vals;

    vars =
        cons(contextSymbol,
            cons(parametersSymbol,
                cons(codeSymbol,
                    cons(nameSymbol,0))));

    vals = cons(env, rest); 

    return cons(closureSymbol,cons(vars,cons(vals,0)));
    }

int
makeBuiltIn(int env,int parameters,int body,int name)
    {
    int vars,vals;

    vars =
        cons(contextSymbol,
            cons(parametersSymbol,
                cons(codeSymbol,
                    cons(nameSymbol,0))));

    vals = 
        cons(env,
            cons(parameters,
                cons(body,
                    cons(name,0))));

    printf("adding %d (%d)\n",ival(name),SymbolCount);
    printf("adding %s\n",SymbolTable[ival(name)]);
    printf("adding %s to <object %d>\n",SymbolTable[ival(name)],env);
    return defineVariable(name,cons(builtInSymbol,cons(vars,cons(vals,0))),env);
    }

int
makeObject(int context,int dynamic,int constructor,int vars,int vals)
    {
    int o;

    vars = 
        cons(contextSymbol,
            cons(dynamicContextSymbol,
                cons(constructorSymbol,
                    cons(thisSymbol,vars))));
    vals =
        cons(context,
            cons(dynamic,
                cons(constructor,
                    cons(0,vals))));

    o = cons(objectSymbol,cons(vars,cons(vals,0)));
    object_this(o) = o;

    return o;
    }

int
makeLambda(int context,int name,int parameters,int body)
    {
    int vars,vals;

    vars = 
        cons(contextSymbol,
            cons(nameSymbol,
                cons(parametersSymbol,
                    cons(codeSymbol,0))));
    vals =
        cons(context,
            cons(name,
                cons(parameters,
                    cons(body,0))));

    return cons(lambdaSymbol,cons(vars,cons(vals,0)));
    }

int
makeError(int tag,int context,int expr,int kind,int value,int trace)
    {
    int vars,vals;

    vars =
        cons(contextSymbol,
            cons(codeSymbol,
                cons(typeSymbol,
                    cons(valueSymbol,
                        cons(traceSymbol,0)))));
    vals =
        cons(context,
            cons(expr,
                cons(kind,
                    cons(value,
                        cons(trace, 0)))));

    return cons(tag,cons(vars,cons(vals,0)));
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
        int vars = car(env);
        int vals = cadr(env);
        while (vars != 0)
            {
            if (ival(car(vars)) == index) return vals;
            vars = cdr(vars);
            vals = cdr(vals);
            }
        env = object_context(env);
        }

   return 0;
   }
