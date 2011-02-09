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

void
loadBuiltIns(int env)
    {
    int count = 0;

    BuiltIns[count] = quote;
    makeBuiltIn(env,
        cons(newString("$item"),0),
        newInteger(count),
        newString("quote"));
    ++count;

    BuiltIns[count] = define;
    makeBuiltIn(env,
        cons(newString("$name"),cons(newString("$"),0)),
        newInteger(count),
        newString("define"));
    ++count;

    BuiltIns[count] = lambda;
    makeBuiltIn(env,
        cons(newString("$params"),cons(newString("$"),0)),
        newInteger(count),
        newString("lambda"));
    ++count;

    BuiltIns[count] = plus;
    makeBuiltIn(env,
        cons(newString("@"),0),
        newInteger(count),
        newString("plus"));
    ++count;

    assert(count <= sizeof(BuiltIns) / sizeof(PRIM));
    }

