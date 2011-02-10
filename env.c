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
makeBuiltIn(int env,int name,int parameters,int body)
    {
    int b = makeClosure(env,name,parameters,body);

    car(b) = builtInSymbol;

    return b;
    }

int
makeObject(int context,int constructor,int vars,int vals)
    {
    int o;

    vars = 
        cons(contextSymbol,
            cons(constructorSymbol,
                cons(thisSymbol,vars)));
    vals =
        cons(context,
            cons(constructor,
                cons(0,vals)));

    o = cons(objectSymbol,cons(vars,cons(vals,0)));
    object_this(o) = o;

    return o;
    }

int
makeClosure(int context,int name,int parameters,int body)
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

    return cons(closureSymbol,cons(vars,cons(vals,0)));
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
