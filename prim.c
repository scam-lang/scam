#include <stdio.h>
#include <assert.h>
#include "types.h"
#include "env.h"
#include "cell.h"
#include "prim.h"
#include "eval.h"
#include "pp.h"
#include "util.h"

PRIM BuiltIns[1000];

static int
quote(int args,int env)
    {
    return thunk_code(car(args));
    }

static int
defineIdentifier(int name,int args,int definingEnv,int initEnv)
    {
    int init;

    if (args == 0)
        init = 0;
    else
        init = eval(car(args),initEnv);

     return defineVariable(name,init,definingEnv);
     }
        
static int
defineFunction(int name,int parameters,int body,int env)
    {
    int closure = makeClosure(env,name,parameters,cons(beginSymbol,body));

    return defineVariable(name,closure,env);
    }
        
static int
define(int args,int env)
    {
    int actualArgs = thunk_code(car(args));
    int name = car(actualArgs);
    int rest = cdr(actualArgs);

    if (type(name) == CONS)
        return defineFunction(car(name),cdr(name),rest,env);
    else if (type(name) == SYMBOL)
        return defineIdentifier(name,rest,env,thunk_context(car(args)));
    else
        {
        Fatal("cannot define a %s\n",type(name));
        return 0;
        }
    }

static int
lambda(int args,int env)
    {
    int name = anonymousSymbol;
    int params = thunk_code(car(args));
    int body = thunk_code(cadr(args));

    return  makeClosure(env,name,params,cons(beginSymbol,body));
    }

static int
rplus(int args,int env,int accum)
    {
    while (args != 0)
        {
        int arg = car(args);
        char *t = type(arg);
        if (t == INTEGER)
            rval(accum) += ival(arg);
        else if (t == REAL)
            rval(accum) += rval(arg);
        else
            Fatal("wrong type for '+': %s\n",type(car(args)));
        args = cdr(args);
        }

    return accum;
    }

static int
iplus(int args,int env,int accum)
    {
    while (args != 0)
        {
        char *t = type(car(args));

        if (t == REAL)
            {
            type(accum) = REAL;
            rval(accum) = ival(accum);
            return rplus(args,env,accum);
            }
        else if (t == INTEGER)
            ival(accum) += ival(car(args));
        else
            Fatal("wrong type for '+': %s\n",type(car(args)));
            
        args = cdr(args);
        }

    return accum;
    }

static int
plus(int args,int env)
    {
    char *t;

    if (args == 0) return zero;

    t = type(car(args));

    if (t == INTEGER) return iplus(args,env,newInteger(0));
    if (t == REAL) return rplus(args,env,newReal(0));
    
    Fatal("wrong type for '+': %s\n",t);

    return 0;
    }

static int
rminus(int args,int env,int accum)
    {
    while (args != 0)
        {
        int arg = car(args);
        char *t = type(arg);
        if (t == INTEGER)
            rval(accum) -= ival(arg);
        else if (t == REAL)
            rval(accum) -= rval(arg);
        else
            Fatal("wrong type for '-': %s\n",type(car(args)));
        args = cdr(args);
        }

    return accum;
    }

static int
iminus(int args,int env,int accum)
    {
    while (args != 0)
        {
        char *t = type(car(args));

        if (t == REAL)
            {
            type(accum) = REAL;
            rval(accum) = ival(accum);
            return rminus(args,env,accum);
            }
        else if (t == INTEGER)
            ival(accum) -= ival(car(args));
        else
            Fatal("wrong type for '-': %s\n",type(car(args)));
            
        args = cdr(args);
        }

    return accum;
    }

static int
minus(int args,int env)
    {
    char *t;
    int accum;

    if (args == 0) return zero;

    t = type(car(args));

    if (t == INTEGER)
        {
        accum = newInteger(ival(car(args)));
        return iminus(cdr(args),env,accum);
        }
    if (t == REAL)
        {
        accum = newReal(rval(car(args)));
        return rminus(cdr(args),env,accum);
        }
    
    Fatal("wrong type for '-': %s\n",t);

    return 0;
    }

static int
lessThanLoop(int first,int remaining)
    {
    int next;
    char *firstType;
    char *nextType;

    if (remaining == 0) return trueSymbol;

    next = car(remaining);
    firstType = type(first);
    nextType = type(next);

    if (firstType == INTEGER && nextType == INTEGER)
        {
        if (ival(first) < ival(next))
            return lessThanLoop(next,cdr(remaining));
        else
            return falseSymbol;
        }
    else if (firstType == INTEGER && nextType == REAL)
        {
        if (ival(first) < rval(next))
            return lessThanLoop(next,cdr(remaining));
        else
            return falseSymbol;
        }
    else if (firstType == REAL && nextType == INTEGER)
        {
        if (rval(first) < ival(next))
            return lessThanLoop(next,cdr(remaining));
        else
            return falseSymbol;
        }
    else if (firstType == REAL && nextType == REAL)
        {
        if (rval(first) < rval(next))
            return lessThanLoop(next,cdr(remaining));
        else
            return falseSymbol;
        }
    else
        {
        Fatal("wrong types for '<': %s and %s\n",firstType,nextType);
        return 0;
        }
    }

static int
lessThan(int args,int env)
    {
    int result;
    if (args == 0) return trueSymbol;

    pp(stdout,args); printf(": more than one arg, calling lessThanLoop\n");

    result = lessThanLoop(car(args),cdr(args));
    pp(stdout,result); printf(" is the result\n");

    return result;
    }

static int
begin(int args,int env)
    {
    int items = thunk_code(car(args));
    int context = thunk_context(car(args));
    printf("in begin...\n");
    pp(stdout,args);
    while (cdr(items) != 0)
        {
        printf("in begin...\n");
        eval(car(items),context);
        items = cdr(items);
        }
    return makeThunk(car(items),context);
    }

static int
print(int args,int env)
    {
    int last = 0;

    while (args != 0)
        {
        last = car(args);
        pp(stdout,car(args));
        args = cdr(args);
        }

    return last;
    }

static int
println(int args,int env)
    {
    int result = print(args,env);
    fprintf(stdout,"\n");
    return result;
    }

static int
iff(int args,int env)
    {
    int test,then,rest,otherwise;
    
    test = car(args);
    args = cdr(args);
    then = car(args);
    args = cdr(args);
    rest = car(args);

    pp(stdout,test); printf(" is the test");

    otherwise = thunk_code(rest);

    if (sameSymbol(test,trueSymbol))
        {
        printf("if test is true\n");
        return then;
        }
    else
        {
        printf("if test is false\n");
        if (otherwise != 0)
            return makeThunk(car(otherwise),thunk_context(rest));
        else
            return 0;
        }
    }

void
loadBuiltIns(int env)
    {
    int b;
    int count = 0;

    BuiltIns[count] = quote;
    b = makeBuiltIn(env,
        newSymbol("quote"),
        cons(newSymbol("$item"),0),
        newInteger(count));
    defineVariable(closure_name(b),b,env);
    ++count;

    BuiltIns[count] = define;
    b = makeBuiltIn(env,
        newSymbol("define"),
        cons(dollarSymbol,0),
        newInteger(count));
    defineVariable(closure_name(b),b,env);
    ++count;

    BuiltIns[count] = lambda;
    b = makeBuiltIn(env,
        newSymbol("lambda"),
        cons(newSymbol("$params"),cons(dollarSymbol,0)),
        newInteger(count));
    defineVariable(closure_name(b),b,env);
    ++count;

    BuiltIns[count] = plus;
    b = makeBuiltIn(env,
        newSymbol("+"),
        cons(atSymbol,0),
        newInteger(count));
    defineVariable(closure_name(b),b,env);
    ++count;

    BuiltIns[count] = minus;
    b = makeBuiltIn(env,
        newSymbol("-"),
        cons(atSymbol,0),
        newInteger(count));
    defineVariable(closure_name(b),b,env);
    ++count;


    BuiltIns[count] = begin;
    b = makeBuiltIn(env,
        beginSymbol,
        cons(dollarSymbol,0),
        newInteger(count));
    defineVariable(closure_name(b),b,env);
    ++count;

    BuiltIns[count] = print;
    b = makeBuiltIn(env,
        newSymbol("print"),
        cons(atSymbol,0),
        newInteger(count));
    defineVariable(closure_name(b),b,env);
    ++count;

    BuiltIns[count] = println;
    b = makeBuiltIn(env,
        newSymbol("println"),
        cons(atSymbol,0),
        newInteger(count));
    defineVariable(closure_name(b),b,env);
    ++count;

    BuiltIns[count] = iff;
    b = makeBuiltIn(env,
        newSymbol("if"),
        cons(newSymbol("test"),cons(newSymbol("$then"),cons(dollarSymbol,0))),
        newInteger(count));
    defineVariable(closure_name(b),b,env);
    ++count;

    BuiltIns[count] = lessThan;
    b = makeBuiltIn(env,
        newSymbol("<"),
        cons(atSymbol,0),
        newInteger(count));
    defineVariable(closure_name(b),b,env);
    ++count;

    assert(count <= sizeof(BuiltIns) / sizeof(PRIM));
    }

