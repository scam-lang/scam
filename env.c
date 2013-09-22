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
        //extern void displayStack(void);
        //ppObject(stdout,env,0);
        //printf("mem: %s: %d\n",type(var),var);
        //displayStack();
        //debug("var: ",var);
        //debug("env: ",env);
        //debug("env: ",env_context(env));
        //debug("env: ",env_context(env_context(env)));
        //debug("env: ",env_context(env_context(env_context(env))));
        //assert(0);
        return throw(undefinedVariableSymbol,
            "file %s,line %d: "
            "variable %s is undefined",
            SymbolTable[file(var)],line(var),
            SymbolTable[ival(var)]);
        }
    if (sameSymbol(car(spot),uninitializedSymbol))
        {
        return throw(uninitializedVariableSymbol,
            "file %s,line %d: "
            "variable %s is uninitialized",
            SymbolTable[file(var)],line(var),
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
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "Variable %s is undefined",
            SymbolTable[file(var)],line(var),
            SymbolTable[ival(var)]);
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

    assureMemory("defineVariable",DEFINE_CELLS,&env,&var,&val,(int *)0);

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

    assert(ENV_CELLS == OBJECT_CELLS + 8 + 1);

    assureMemory("makeEnvironment",ENV_CELLS,
        &context,&constructor,&vars,&vals,(int *)0);

    o = makeObject(envSymbol);

    object_variable_hook(o) =
        ucons(contextSymbol,
            ucons(levelSymbol,
                ucons(constructorSymbol,
                    ucons(thisSymbol,vars))));

    object_value_hook(o) =
        ucons(context,
            ucons(context == 0? newInteger(0) : env_level(context),
                ucons(constructor,
                    ucons(o,vals))));

    return o;
    }

int
makeThunk(int expr,int env)
    {
    int o;

    assert(THUNK_CELLS == OBJECT_CELLS + 4);

    assureMemory("makeThunk",THUNK_CELLS,&expr,&env,(int *)0);

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
        &context,&name,&parameters,&body,(int *)0);

    o = makeObject(closureSymbol);

    if (mode == ADD_BEGIN)
       {
       //debug("adding body to",name);
       //printf("    from %s,line %d\n",
           //SymbolTable[file(body)],
           //line(body));
       body = uconsfl(beginSymbol,body,file(body),line(body));
       }

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
makeError(int tag,int code,int value,int trace)
    {
    int o;

    assert(ERROR_CELLS == OBJECT_CELLS + 6);

    assureMemory("makeError",ERROR_CELLS,
        &tag,&code,&value,&trace,(int *)0);

    o = makeObject(tag);

    object_variable_hook(o) =
        ucons(codeSymbol,
            ucons(valueSymbol,
                ucons(traceSymbol,0)));

    object_value_hook(o) =
        ucons(code,
            ucons(value,
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
makeThrow(int code,int value,int trace)
    {
    return makeError(throwSymbol,code,value,trace);
    }

int
throw(int symbol,char *fmt, ...)
    {
    va_list ap;
    int s,result;
    char buffer[512];

    //printf("encountered a fatal error...\n");

    va_start(ap, fmt);
    vsnprintf(buffer,sizeof(buffer), fmt, ap);
    va_end(ap);

    s = newString(buffer);

    result = makeThrow(symbol,s,0);
    //debug("throwing",result);
    return result;
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

int
isLocal(int var,int env)
    {
    int vars = object_variables(env);
    int vals = object_values(env);
    while (vars != 0)
        {
        //printf("looking at %s\n",SymbolTable[ival(car(vars))]);
        if (ival(car(vars)) == ival(var)) return 1;
        vars = cdr(vars);
        vals = cdr(vals);
        }

    return 0;
    }
