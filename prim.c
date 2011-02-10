#include <assert.h>
#include "types.h"
#include "env.h"
#include "cell.h"
#include "prim.h"
#include "eval.h"
#include "util.h"

PRIM BuiltIns[1000];

static int
quote(int args,int env)
    {
    return cons(quoteSymbol,cons(car(args),0));
    }

static int
defineIdentifier(int args,int env)
    {
    int init;

    if (cdr(args) == 0)
        init = 0;
    else
        init = eval(cadr(args),env);

     return defineVariable(car(args),init,env);
     }
        
static int
defineFunction(int args,int env)
    {
    int name = caar(args);
    int params = cdr(car(args));
    int body = cdr(args);
    int lambda = makeLambda(env,name,params,body);

     return defineVariable(name,lambda,env);
     }
        
static int
lambda(int args,int env)
    {
    int name = anonymousSymbol;
    int params = car(args);
    int body = cdr(args);

    return  makeLambda(env,name,params,body);
    }

static int
define(int args,int env)
    {
    if (type(car(args)) == CONS)
        return defineFunction(args,env);
    else if (type(car(args)) == SYMBOL)
        return defineIdentifier(args,env);
    else
        {
        Fatal("cannot define a %s\n",type(car(args)));
        return 0;
        }
    }


static int
plus(int args,int env)
    {
    int total;
    if (args == 0) return zero;

    total = 0;
    while (args != 0)
        {
        total += ival(car(args));
        args = cdr(args);
        }

    return newInteger(total);
    }

static int
begin(int args,int env)
    {
    int thunks = car(args);
    while (cdr(thunks) != 0)
        {
        eval(thunk_code(car(thunks)),thunk_context(car(thunks)));
        thunks = cdr(thunks);
        }
    return car(thunks);
    }

void
loadBuiltIns(int env)
    {
    int count = 0;

    BuiltIns[count] = quote;
    makeBuiltIn(env,
        cons(newSymbol("$item"),0),
        newInteger(count),
        newSymbol("quote"));
    ++count;

    BuiltIns[count] = define;
    makeBuiltIn(env,
        cons(newSymbol("$name"),cons(dollarSymbol,0)),
        newInteger(count),
        newSymbol("define"));
    ++count;

    BuiltIns[count] = lambda;
    makeBuiltIn(env,
        cons(newSymbol("$params"),cons(dollarSymbol,0)),
        newInteger(count),
        newSymbol("lambda"));
    ++count;

    BuiltIns[count] = plus;
    makeBuiltIn(env,
        cons(atSymbol,0),
        newInteger(count),
        newSymbol("plus"));
    ++count;

    BuiltIns[count] = begin;
    makeBuiltIn(env,
        cons(dollarSymbol,0),
        newInteger(count),
        newSymbol("begin"));
    ++count;

    assert(count <= sizeof(BuiltIns) / sizeof(PRIM));
    }

