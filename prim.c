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
quote(int args)
    {
    return thunk_code(car(args));
    }

/* (define #) */

static int
defineIdentifier(int name,int init,int env)
    {
    if (init != 0)
        {
        push(name);
        push(env);
        init = eval(init,env);
        env = pop();
        name = pop();
        }

     assureMemory("defineIdentifier",DEFINE_CELLS,&env,&name,&init,0);

     return defineVariable(env,name,init);
     }
        
static int
defineFunction(int name,int parameters,int body,int env)
    {
    int closure;
    
    assureMemory("defineFunction",CLOSURE_CELLS + DEFINE_CELLS,&name,&parameters,&body,&env,0);

    closure = makeClosure(env,name,parameters,body,ADD_BEGIN);

    return defineVariable(env,name,closure);
    }

/* (define #) */

static int
define(int args)
    {
    int actualArgs = thunk_code(car(args));
    int first = car(actualArgs);
    int rest = cdr(actualArgs);

    if (type(first) == CONS)
        return defineFunction(car(first),cdr(first),rest,
            thunk_context(car(args)));
    else if (type(first) == SYMBOL)
        return defineIdentifier(first,car(rest),
            thunk_context(car(args)));
    else
        return Fatal("cannot define a %s\n",type(first));
    }

/* (lambda $params #) */

static int
lambda(int args)
    {
    int name = anonymousSymbol;
    int params = thunk_code(car(args));
    int body = thunk_code(cadr(args));

    return  makeClosure(thunk_context(car(args)),name,params,body,ADD_BEGIN);
    }

static int
rplus(int args,int accum)
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
            Fatal("real addition: wrong type for '+': %s\n",type(car(args)));
        args = cdr(args);
        }

    return accum;
    }

static int
iplus(int args,int accum)
    {
    while (args != 0)
        {
        char *t = type(car(args));

        if (t == REAL)
            {
            type(accum) = REAL;
            rval(accum) = ival(accum);
            return rplus(args,accum);
            }
        else if (t == INTEGER)
            ival(accum) += ival(car(args));
        else
            {
            pp(stdout,car(args));
            Fatal("integer addition: wrong type for '+': %s\n",type(car(args)));
            }
            
        args = cdr(args);
        }

    //debug("iplus",accum);
    return accum;
    }

/* (+ @) */

static int
plus(int args)
    {
    char *t;

    args = car(args);

    if (args == 0) return zero;

    assureMemory("plus",1,&args,0);

    t = type(car(args));

    if (t == INTEGER) return iplus(args,newInteger(0));
    if (t == REAL) return rplus(args,newReal(0));
    
    Fatal("addition: wrong type for '+': %s\n",t);

    return 0;
    }

static int
rminus(int args,int accum)
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
iminus(int args,int accum)
    {
    while (args != 0)
        {
        char *t = type(car(args));

        if (t == REAL)
            {
            type(accum) = REAL;
            rval(accum) = ival(accum);
            return rminus(args,accum);
            }
        else if (t == INTEGER)
            ival(accum) -= ival(car(args));
        else
            Fatal("wrong type for '-': %s\n",type(car(args)));
            
        args = cdr(args);
        }

    return accum;
    }

/* (- @) */

static int
minus(int args)
    {
    char *t;
    int accum;

    args = car(args);

    if (args == 0) return zero;

    assureMemory("minus",1,&args,0);

    t = type(car(args));

    if (t == INTEGER)
        {
        accum = newInteger(ival(car(args)));
        return iminus(cdr(args),accum);
        }
    if (t == REAL)
        {
        accum = newReal(rval(car(args)));
        return rminus(cdr(args),accum);
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

    //debug("first",first);

    if (remaining == 0) return trueSymbol;

    next = car(remaining);
    firstType = type(first);
    nextType = type(next);

    //debug("next",next);

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
lessThan(int args)
    {
    int result;
    if (args == 0) return trueSymbol;

    //debug("lessThan args",args);

    result = lessThanLoop(caar(args),cdar(args));
    //pp(stdout,result); printf(" is the result\n");

    return result;
    }

/* (begin $) */

static int
begin(int args)
    {
    //printf("in begin...\n");
    return evalThunkListExceptLast(car(args));
    }

/* (scope #) */

static int
scope(int args)
    {
    int result,context,env;
    //printf("in scope...\n");
    assureMemory("scope",OBJECT_CELLS,&args,0);
    context = thunk_context(car(args));
    env = makeObject(context,0,0,0);
    result = evalListExceptLast(thunk_code(car(args)),env);
    return result;
    }

/* (print @) */

static int
print(int args)
    {
    int last = 0;

    args = car(args);

    while (args != 0)
        {
        last = car(args);
        pp(stdout,car(args));
        args = cdr(args);
        }

    return last;
    }

static int
println(int args)
    {
    int result = print(args);
    fprintf(stdout,"\n");
    return result;
    }

/* (if test $then $) */

static int
iif(int args)
    {
    //debug("if test",car(args));

    if (sameSymbol(car(args),trueSymbol))
        {
        //printf("if test is true\n");
        //debug("then",cadr(args));
        return cadr(args);
        }
    else
        {
        //printf("if test is false\n");
        int otherwise = caddr(args);
        if (otherwise != 0)
            {
            //debug("else",cadr(args));
            return car(otherwise);
            }
        else
            return 0;
        }
    }

/* (while $test $) */

static int
wwhile(int args)
    {
    int last = 0;
    int testResult;
    
    //printf("in while...\n");

    push(args);
    testResult = eval(thunk_code(car(args)),thunk_context(car(args)));
    args = pop();

    while (sameSymbol(testResult,trueSymbol))
        {
        push(args);
        last = evalThunkList(cadr(args));
        args = pop();

        push(args);
        testResult = eval(thunk_code(car(args)),thunk_context(car(args)));
        args = pop();

        //debug("test result",testResult);
        }

    return last;
    }

/* (set! $var value @) */

static int
set(int args)
    {
    int var = car(args);
    int expr = thunk_code(var);
    int at;
    
    //printf("in set!...");
    if (type(expr) != SYMBOL)
        {
        push(args);
        expr = eval(expr,thunk_context(var));
        args = pop();
        if (type(expr) != SYMBOL)
            return Fatal("set!: variable argument resolved to %s, not SYMBOL\n",
                type(expr));
        }

    at = caddr(args);

    //debug("new value",cadr(args));
    if (at == 0)
        return setVariableValue(expr,cadr(args),thunk_context(car(args)));
    else
        return setVariableValue(expr,cadr(args),car(at));
    }

/* (get $var @) */

static int 
get(int args)
    {
    int var = car(args);
    int expr = thunk_code(var);
    int at;

    //printf("in get...");
    if (type(expr) != SYMBOL)
        {
        push(args);
        expr = eval(expr,thunk_context(var));
        args = pop();
        if (type(expr) != SYMBOL)
            return Fatal("get: variable argument resolved to %s, not SYMBOL\n",
                type(expr));
        }

    at = cadr(args);
    //ppObject(stdout,thunk_context(car(args)),0);
    //debug("at",at);

    if (at == 0)
        return getVariableValue(expr,thunk_context(car(args)));
    else
        return getVariableValue(expr,car(at));
    }

static int
force(int args)
    {
    return car(args);
    }

static int
eeval(int args)
    {
    return eval(car(args),cadr(args));
    }

static int
ccar(int args)
    {
    return car(car(args));
    }

static int
ccdr(int args)
    {
    return cdr(car(args));
    }

void
loadBuiltIns(int env)
    {
    int b;
    int count = 0;

    BuiltIns[count] = ccdr;
    b = makeBuiltIn(env,
        newSymbol("cdr"),
        ucons(newSymbol("items"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ccar;
    b = makeBuiltIn(env,
        newSymbol("car"),
        ucons(newSymbol("items"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = eeval;
    b = makeBuiltIn(env,
        newSymbol("eval"),
        ucons(newSymbol("expr"),
            ucons(newSymbol("context"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = force;
    b = makeBuiltIn(env,
        newSymbol("force"),
        ucons(newSymbol("$thunk"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = scope;
    b = makeBuiltIn(env,
        newSymbol("scope"),
        ucons(sharpSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = begin;
    b = makeBuiltIn(env,
        beginSymbol,
        ucons(dollarSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = define;
    b = makeBuiltIn(env,
        newSymbol("define"),
        ucons(sharpSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = print;
    b = makeBuiltIn(env,
        newSymbol("print"),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = println;
    b = makeBuiltIn(env,
        newSymbol("println"),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = plus;
    b = makeBuiltIn(env,
        newSymbol("+"),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = quote;
    b = makeBuiltIn(env,
        newSymbol("quote"),
        ucons(newSymbol("$item"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = lambda;
    b = makeBuiltIn(env,
        newSymbol("lambda"),
        ucons(newSymbol("$params"),
            ucons(sharpSymbol,0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = minus;
    b = makeBuiltIn(env,
        newSymbol("-"),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = iif;
    b = makeBuiltIn(env,
        newSymbol("if"),
        ucons(newSymbol("test"),
            ucons(newSymbol("$then"),
                ucons(dollarSymbol,0))),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = wwhile;
    b = makeBuiltIn(env,
        newSymbol("while"),
        ucons(newSymbol("$test"),
            ucons(dollarSymbol,0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;


    BuiltIns[count] = lessThan;
    b = makeBuiltIn(env,
        newSymbol("<"),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = get;
    b = makeBuiltIn(env,
        newSymbol("get"),
        ucons(newSymbol("$var"),
            ucons(newSymbol("@"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = set;
    b = makeBuiltIn(env,
        newSymbol("set!"),
        ucons(newSymbol("$var"),
            ucons(newSymbol("value"),
                ucons(newSymbol("@"),0))),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;


    assert(count <= sizeof(BuiltIns) / sizeof(PRIM));
    }
