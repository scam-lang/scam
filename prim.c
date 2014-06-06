
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
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "scam.h"
#include "types.h"
#include "env.h"
#include "cell.h"
#include "parser.h"
#include "prim.h"
#include "eval.h"
#include "thread.h"

PRIM BuiltIns[1000];
FILE *OpenPorts[20];
char *OpenPortNames[20];
int MaxPorts = sizeof(OpenPorts) / sizeof(FILE *);
int CurrentInputIndex;
int CurrentOutputIndex;

static int
scamBoolean(int item)
    {
    if (item == 0)
        return FalseSymbol;
    else
        return TrueSymbol;
    }

/* (quote $item) */

static int
quote(int args)
    {
    return car(args);
    }

/*
 *  defineIdentifier    -   Creates a new variable in the enviroment
 *
 *  Not heap safe
 */

static int
defineIdentifier(int name,int init,int env)
    {
    if (init == 0)
        init = UninitializedSymbol;
    else
        {
        PUSH(name);
        PUSH(env);
        init = eval(car(init),env);
        env = POP();
        name = POP();

        rethrow(init);
        }

    P();
    ENSURE_MEMORY(DEFINE_VARIABLE_SIZE,&env,&name,&init,(int *) 0);
    int o = defineVariable(env,name,init);
    V();
    return o;
    }
    
/*
 *  defineFunction  -   Define a new function in the environment
 *
 *  Not heap safe
 */

static int
defineFunction(int name,int parameters,int body,int env)
    {
    int spot;
    int closure;
    
    /* Safe */
    spot = parameters;
    while (spot != 0)
        {
        int sym = car(spot);
        if (type(sym) != SYMBOL)
            {
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "only SYMBOLS can be parameters, found type %s",
                SymbolTable[file(name)],
                line(name),
                type(sym));
            }
        spot = cdr(spot);
        }
   
    P();
    ENSURE_MEMORY(DEFINE_VARIABLE_SIZE + MAKE_CLOSURE_SIZE,
        &env,&name,&parameters,&body,(int *) 0);

    closure = makeClosure(env,name,parameters,body,ADD_BEGIN);
    int o = defineVariable(env,name,closure); 
    V();

    return o; 
    }

/* (define # $) */

static int
define(int args)
    {
    int actualArgs = cadr(args);
    int first,rest;
    
    if (actualArgs == 0)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "not enough arguments for definition",
            SymbolTable[file(args)],
            line(args)
            );
        }

    first = car(actualArgs);
    rest = cdr(actualArgs);

    char* TYPE = type(first);
    if (TYPE == CONS)
        {
        return defineFunction(
                car(first),
                cdr(first),
                rest,
                car(args)
            );
        }
    else if (TYPE == SYMBOL)
        {
        return defineIdentifier(
                first,
                rest,
                car(args)
            );
        }
    else
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "can only define SYMBOLS, not type %s",
            SymbolTable[file(args)],
            line(args),
            TYPE);
        }
    }

/* (addSymbol name init env) */

static int
addSymbol(int args)
    {
    P();
    ENSURE_MEMORY(DEFINE_VARIABLE_SIZE,&args,(int *) 0);

    int o = defineVariable(
            caddr(args),
            car(args),
            cadr(args));
    V();
    return o;
    }

/* (lambda # $params $) */

static int
lambda(int args)
    {
    P();
    ENSURE_MEMORY(MAKE_CLOSURE_SIZE,&args,(int *) 0);

    int name = AnonymousSymbol;
    int params = cadr(args);
    int body = caddr(args);
    int o = makeClosure(
            car(args),
            name,
            params,
            body,
            ADD_BEGIN
            );
    V();
    return o;
    }

/* (closure name params body env) */

static int
closure(int args)
    {
    P();
    ENSURE_MEMORY(MAKE_CLOSURE_SIZE,&args,(int *) 0);

    int name = cadr(args);
    int params = caddr(args);
    int body = cadddr(args);
    int env = caddddr(args);
    int o = makeClosure(
            env,
            name,
            params,
            body,
            ADD_BEGIN
            );
    V();
    return o;
    }


static int
not(int args)
    {
    int SYM = car(args);
    if (SameSymbol(SYM,TrueSymbol))
        {
        return FalseSymbol;
        }

    if (SameSymbol(SYM,FalseSymbol))
        {
        return TrueSymbol;
        }

    return throw(ExceptionSymbol,
            "file %s,line %d: "
            "cannot logically negate type %s",
            SymbolTable[file(args)],
            line(args),
            type(car(args))
        );
    }

/* (< a b) */

static int
isLessThan(int args)
    {
    int a,b,result;
    char *aType,*bType;

    args = car(args);

    if (args == 0)
        {
        return scamBoolean(1);
        }

    if (cdr(args) == 0)
        {
        return scamBoolean(1);
        }

    a = car(args);
    args = cdr(args);

    result = 1;

    while (args != 0)
        {
        aType = type(a);
        b = car(args);
        bType = type(b);

        if (aType == INTEGER && bType == INTEGER)
            {
            result = ival(a) < ival(b);
            }
        else if (aType == INTEGER && bType == REAL)
            {
            result = ival(a) < rval(b);
            }
        else if (aType == REAL && bType == INTEGER)
            {
            result = rval(a) < ival(b);
            }
        else if (aType == REAL && bType == REAL)
            {
            result = rval(a) < rval(b);
            }
        else
            {
            return throw(
                    ExceptionSymbol,
                    "file %s,line %d: "
                    "wrong types for '<': %s and %s",
                    SymbolTable[file(args)],
                    line(args),
                    aType,bType
                );
            }

        if (!result) 
            {
            break;
            }

        args = cdr(args);
        a = b;
        }

    result = scamBoolean(result);

    setfile(result,file(args));
    setline(result,line(args));

    return result;
    }

/* (<= a b) */

static int
isLessThanOrEqualTo(int args)
    {
    int a,b,result;
    char *aType,*bType;

    args = car(args);

    if (args == 0)
        {
        return scamBoolean(1);
        }

    if (cdr(args) == 0)
        {
        return scamBoolean(1);
        }

    a = car(args);
    args = cdr(args);

    result = 1;

    while (args != 0)
        {
        aType = type(a);
        b = car(args);
        bType = type(b);

        if (aType == INTEGER && bType == INTEGER)
            {
            result = ival(a) <= ival(b);
            }
        else if (aType == INTEGER && bType == REAL)
            {
            result = ival(a) <= rval(b);
            }
        else if (aType == REAL && bType == INTEGER)
            {
            result = rval(a) <= ival(b);
            }
        else if (aType == REAL && bType == REAL)
            {
            result = rval(a) <= rval(b);
            }
        else
            {
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "wrong types for '<=': %s and %s",
                SymbolTable[file(args)],line(args),
                aType,bType);
            }

        if (!result)
            {
            break;
            }

        args = cdr(args);
        a = b;
        }

    result = scamBoolean(result);

    setfile(result,file(args));
    setline(result,line(args));

    return result;
    }

/* (= a b) */

static int
isNumericEqualTo(int args)
    {
    int a,b,result;
    char *aType,*bType;

    args = car(args);

    if (args == 0) return scamBoolean(1);

    if (cdr(args) == 0) return scamBoolean(1);

    a = car(args);
    args = cdr(args);

    result = 1;

    while (args != 0)
        {
        aType = type(a);
        b = car(args);
        bType = type(b);

        if (aType == INTEGER && bType == INTEGER)
            result = ival(a) == ival(b);
        else if (aType == INTEGER && bType == REAL)
            result = ival(a) == rval(b);
        else if (aType == REAL && bType == INTEGER)
            result = rval(a) == ival(b);
        else if (aType == REAL && bType == REAL)
            result = rval(a) == rval(b);
        else
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "wrong types for '=': %s and %s",
                SymbolTable[file(args)],line(args),
                aType,bType);

        if (!result) break;

        args = cdr(args);
        a = b;
        }

    result = scamBoolean(result);
 
    setfile(result,file(args));
    setline(result,line(args));

    return result;
    }

/* (> a b) */

static int
isGreaterThan(int args)
    {
    int a,b,result;
    char *aType,*bType;

    args = car(args);

    if (args == 0) return scamBoolean(1);

    if (cdr(args) == 0) return scamBoolean(1);

    a = car(args);
    args = cdr(args);

    result = 1;

    while (args != 0)
        {
        aType = type(a);
        b = car(args);
        bType = type(b);

        if (aType == INTEGER && bType == INTEGER)
            result = ival(a) > ival(b);
        else if (aType == INTEGER && bType == REAL)
            result = ival(a) > rval(b);
        else if (aType == REAL && bType == INTEGER)
            result = rval(a) > ival(b);
        else if (aType == REAL && bType == REAL)
            result = rval(a) > rval(b);
        else
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "wrong types for '>': %s and %s",
                SymbolTable[file(args)],line(args),
                aType,bType);

        if (!result) break;

        args = cdr(args);
        a = b;
        }

    result = scamBoolean(result);

    setfile(result,file(args));
    setline(result,line(args));

    return result;
    }


/* (>= a b) */

static int
isGreaterThanOrEqualTo(int args)
    {
    int a,b,result;
    char *aType,*bType;

    args = car(args);

    if (args == 0) return scamBoolean(1);

    if (cdr(args) == 0) return scamBoolean(1);

    a = car(args);
    args = cdr(args);

    result = 1;

    while (args != 0)
        {
        aType = type(a);
        b = car(args);
        bType = type(b);

        if (aType == INTEGER && bType == INTEGER)
            result = ival(a) >= ival(b);
        else if (aType == INTEGER && bType == REAL)
            result = ival(a) >= rval(b);
        else if (aType == REAL && bType == INTEGER)
            result = rval(a) >= ival(b);
        else if (aType == REAL && bType == REAL)
            result = rval(a) >= rval(b);
        else
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "wrong types for '>=': %s and %s",
                SymbolTable[file(args)],line(args),
                aType,bType);

        if (!result) break;

        args = cdr(args);
        a = b;
        }

    
    result = scamBoolean(result);
    setfile(result,file(args));
    setline(result,line(args));

    return result;
    }

/* (eq? a b) */

static int
isEq(int args)
    {
    int a,b,result;
    char *aType;

    args = car(args);

    if (args == 0) return scamBoolean(1);

    if (cdr(args) == 0) return scamBoolean(1);

    a = car(args);
    args = cdr(args);

    result = 1;

    while (args != 0)
        {
        aType = type(a);
        b = car(args);
 
        if (type(a) != type(b))
            result = 0;
        else if (aType == INTEGER)
            result = ival(a) == ival(b);
        else if (aType == REAL)
            result = rval(a) == rval(b);
        else if (aType == SYMBOL)
            result = ival(a) == ival(b);
        else 
            result = a == b;

        if (!result) break;

        args = cdr(args);
        a = b;
        }

    result = scamBoolean(result);

    setfile(result,file(args));
    setline(result,line(args));

    return result;
    }

/* (neq? a b) */

static int
isNotEq(int args)
    {
    int a,b,result;
    char *aType;

    args = car(args);

    if (args == 0) return scamBoolean(1);

    if (cdr(args) == 0) return scamBoolean(1);

    a = car(args);
    args = cdr(args);

    result = 1;

    while (args != 0)
        {
        aType = type(a);
        b = car(args);
 
        if (type(a) != type(b))
            result = 1;
        else if (aType == INTEGER)
            result = ival(a) != ival(b);
        else if (aType == REAL)
            result = rval(a) != rval(b);
        else if (aType == SYMBOL)
            result = ival(a) != ival(b);
        else 
            result = a != b;

        if (!result) break;

        args = cdr(args);
        a = b;
        }

    result = scamBoolean(result);

    setfile(result,file(args));
    setline(result,line(args));

    return result;
    }

/* (+ @) */

static int
plus(int args)
    {
    int a,b,result;
    int itotal = 0;
    double rtotal = 0;
    char *oldType,*newType;

    args = car(args);

    if (args == 0) return newInteger(0);

    a = car(args);
    args = cdr(args);

    oldType = type(a);
    itotal = ival(a);
    rtotal = rval(a);


    while (args != 0)
        {
        b = car(args);
        newType = type(b);

        if (oldType == INTEGER && newType == INTEGER)
            itotal += ival(b);
        else if (oldType == INTEGER && newType == REAL)
            {
            rtotal = itotal + rval(b);
            oldType = REAL;
            }
        else if (oldType == REAL && newType == INTEGER)
            rtotal += ival(b);
        else if (oldType == REAL && newType == REAL)
            rtotal += rval(b);
        else
            {
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "wrong types for '+': %s and %s",
                SymbolTable[file(args)],line(args),
                oldType,newType == SYMBOL? SymbolTable[ival(b)] : newType);
            }
        args = cdr(args);
        }

    int f = file(args);
    int l = line(args);

    if (oldType == INTEGER)
        result = newInteger(itotal);
    else if (oldType == REAL)
        result = newReal(rtotal);
    else
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "wrong types for '+': %s",
            SymbolTable[file(args)],line(args),
            oldType);
        }

    setfile(result,f);
    setline(result,l);

    return result;
    }

/* (- @) */

static int
minus(int args)
    {
    int a,b,result;
    int itotal = 0;
    double rtotal = 0;
    char *oldType,*newType;

    args = car(args);

    if (args == 0) return newInteger(0);

    a = car(args);
    args = cdr(args);


    oldType = type(a);
    itotal = ival(a);
    rtotal = rval(a);

    if (args == 0) 
        {
        if (oldType == INTEGER) return newInteger(0 - itotal);
        if (oldType == REAL) return newReal(0 - rtotal);
        }

    while (args != 0)
        {
        b = car(args);
        newType = type(b);

        if (oldType == INTEGER && newType == INTEGER)
            itotal -= ival(b);
        else if (oldType == INTEGER && newType == REAL)
            {
            rtotal = itotal - rval(b);
            oldType = REAL;
            }
        else if (oldType == REAL && newType == INTEGER)
            rtotal -= ival(b);
        else if (oldType == REAL && newType == REAL)
            rtotal -= rval(b);
        else
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "wrong types for '-': %s and %s",
                SymbolTable[file(args)],line(args),
                oldType,newType);

        args = cdr(args);
        }

    int f = file(args);
    int l = line(args);

    if (oldType == INTEGER)
        result = newInteger(itotal);
    else if (oldType == REAL)
        result = newReal(rtotal);
    else
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "wrong types for '-': %s",
            SymbolTable[file(args)],line(args),
            oldType);

    setfile(result,f);
    setline(result,l);

    return result;
    }

/* (* @) */

static int
times(int args)
    {
    int a,b,result;
    int itotal = 0;
    double rtotal = 0;
    char *oldType,*newType;

    args = car(args);

    if (args == 0) return newInteger(1);

    a = car(args);
    args = cdr(args);

    oldType = type(a);
    itotal = ival(a);
    rtotal = rval(a);

    while (args != 0)
        {
        b = car(args);
        newType = type(b);

        if (oldType == INTEGER && newType == INTEGER)
            itotal *= ival(b);
        else if (oldType == INTEGER && newType == REAL)
            {
            rtotal = itotal * rval(b);
            oldType = REAL;
            }
        else if (oldType == REAL && newType == INTEGER)
            rtotal *= ival(b);
        else if (oldType == REAL && newType == REAL)
            rtotal *= rval(b);
        else
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "wrong types for '*': %s and %s",
                SymbolTable[file(args)],line(args),
                oldType,newType);

        args = cdr(args);
        }

    int f = file(args);
    int l = line(args);

    if (oldType == INTEGER)
        result = newInteger(itotal);
    else if (oldType == REAL)
        result = newReal(rtotal);
    else
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "wrong types for '*': %s",
            SymbolTable[file(args)],line(args),
            oldType);

    setfile(result,f);
    setline(result,l);

    return result;
    }

/* (/ @) */

static int
divides(int args)
    {
    int a,b,result;
    int itotal = 0;
    double rtotal = 0;
    char *oldType,*newType;

    args = car(args);

    if (args == 0) return newInteger(1);

    a = car(args);
    args = cdr(args);

    oldType = type(a);
    itotal = ival(a);
    rtotal = rval(a);

    while (args != 0)
        {
        b = car(args);
        newType = type(b);

        if ((newType == INTEGER && ival(b) == 0)
        ||  (newType == REAL && rval(b) == 0.0))
            return throw(MathExceptionSymbol,
                "file %s,line %d: "
                "divide by zero",
                SymbolTable[file(args)],line(args)
                );

        if (oldType == INTEGER && newType == INTEGER)
            itotal /= ival(b);
        else if (oldType == INTEGER && newType == REAL)
            {
            rtotal = itotal / rval(b);
            oldType = REAL;
            }
        else if (oldType == REAL && newType == INTEGER)
            rtotal /= ival(b);
        else if (oldType == REAL && newType == REAL)
            rtotal /= rval(b);
        else
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "wrong types for '/': %s and %s",
                SymbolTable[file(args)],line(args),
                oldType,newType);

        args = cdr(args);
        }

    int f = file(args);
    int l = line(args);

    if (oldType == INTEGER)
        result = newInteger(itotal);
    else if (oldType == REAL)
        result = newReal(rtotal);
    else
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "wrong types for '/': %s",
            SymbolTable[file(args)],line(args),
            oldType);

    setfile(result,f);
    setline(result,l);

    return result;
    }

/* (% @) */

static int
mod(int args)
    {
    int a,b,result;
    int itotal = 0;
    char *oldType,*newType;

    args = car(args);

    if (args == 0) return newInteger(0);

    a = car(args);
    args = cdr(args);

    oldType = type(a);
    itotal = ival(a);

    while (args != 0)
        {
        b = car(args);
        newType = type(b);

        if (oldType == INTEGER && newType == INTEGER)
            itotal %= ival(b);
        else
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "wrong types for '/': %s and %s",
                SymbolTable[file(args)],line(args),
                oldType,newType);

        args = cdr(args);
        }

    int f = file(args);
    int l = line(args);
    result = newInteger(itotal);

    setfile(result,f);
    setline(result,l);

    return result;
    }

/* (exp a) */

static int
eexp(int args)
    {
    int a,result;
    char *aType;

    a = car(args);
    aType = type(a);

    int f = file(a);
    int l = line(a);

    if (aType == INTEGER)
        result = newReal(exp(ival(a)));
    else if (aType == REAL)
        result = newReal(exp(rval(a)));
    else
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "wrong type for 'exp': %s",
            SymbolTable[file(car(args))],line(car(args)),
            aType);

    setfile(result,f);
    setline(result,l);

    return result;
    }

/* (log a) */

static int
llog(int args)
    {
    int a,result;
    char *aType;

    a = car(args);
    aType = type(a);

    int f = file(a);
    int l = line(a);

    if (aType == INTEGER)
        result = newReal(log(ival(a)));
    else if (aType == REAL)
        result = newReal(log(rval(a)));
    else
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "wrong type for 'exp': %s",
            SymbolTable[file(car(args))],line(car(args)),
            aType);

    setfile(result,f);
    setline(result,l);

    return result;
    }

static int
randomInt(int args)
    {
    int x = random();
    if (x == RAND_MAX) --x;
    return newInteger(x);
    }

static int
randomMax(int args)
    {
    return newInteger(RAND_MAX);
    }

static int
randomSeed(int args)
    {
    int a = car(args);

    if (type(a) == INTEGER && ival(a) > 0)
        {
        srandom(ival(a));
        return 0;
        }
    
    return throw(ExceptionSymbol,
        "file %s,line %d: "
        "randomSeed: argument must be a positive integer",
        SymbolTable[file(a)],line(a));
    }

/* (sin a) */

static int
ssin(int args)
    {
    int a,result;
    char *aType;

    a = car(args);
    aType = type(a);

    int f = file(a);
    int l = line(a);

    if (aType == INTEGER)
        result = newReal(sin(ival(a)));
    else if (aType == REAL)
        result = newReal(sin(rval(a)));
    else
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "wrong type for 'exp': %s",
            SymbolTable[file(car(args))],line(car(args)),
            aType);

    setfile(result,f);
    setline(result,l);

    return result;
    }

/* (cos a) */

static int
coss(int args)
    {
    int a,result;
    char *aType;

    a = car(args);
    aType = type(a);

    int f = file(a);
    int l = line(a);

    if (aType == INTEGER)
        result = newReal(cos(ival(a)));
    else if (aType == REAL)
        result = newReal(cos(rval(a)));
    else
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "wrong type for 'exp': %s",
            SymbolTable[file(car(args))],line(car(args)),
            aType);

    setfile(result,f);
    setline(result,l);

    return result;
    }

/* (atan base expt) */

static int
aatan(int args)
    {
    int result;
    int a = car(args);
    int b = cadr(args);
    char *atype = type(a);
    char *btype = type(b);

    int f = file(a);
    int l = line(a);

    if (atype == INTEGER && btype == INTEGER)
        result = newReal(atan2(ival(a),ival(b)));
    else if (atype == INTEGER)
        result = newReal(atan2(ival(a),rval(b)));
    else if (btype == INTEGER)
        result = newReal(atan2(rval(a),ival(b)));
    else
        result = newReal(atan2(rval(a),rval(b)));

    setfile(result,f);
    setline(result,l);

    return result;
    }
        
/* (expt base expt) */

static int
expt(int args)
    {
    int result;
    int a = car(args);
    int b = cadr(args);
    char *atype = type(a);
    char *btype = type(b);

    int f = file(a);
    int l = line(a);

    if (atype == INTEGER && btype == INTEGER)
        result = newInteger((int) pow(ival(a),ival(b)));
    else if (atype == INTEGER)
        result = newReal(pow(ival(a),rval(b)));
    else if (btype == INTEGER)
        result = newReal(pow(rval(a),ival(b)));
    else
        result = newReal(pow(rval(a),rval(b)));

    setfile(result,f);
    setline(result,l);

    return result;
    }
        
/* (type item) */

static int
ttype(int args)
    {
    return newSymbol(type(car(args)));
    }

/* (begin # $) */

static int
begin(int args)
    {
    return evalBlock(cadr(args),car(args),ALLBUTLAST);
    }

/* (return # $item) */

static int
rreturn(int args)
    {
    P();
    ENSURE_MEMORY(MAKE_THROW_SIZE+MAKE_THUNK_SIZE,&args,(int *) 0);
    int o = makeThrow(ReturnSymbol,makeThunk(cadr(args),car(args)),0);
    V();
    return o; 
    }

/* (scope # $) */

static int
scope(int args)
    {
    int env;
    P();
    ENSURE_MEMORY(MAKE_ENVIRONMENT_SIZE,&args,(int *) 0);
    env = makeEnvironment(car(args),0,0,0);
    V();
    return evalBlock(cadr(args),env,ALLBUTLAST);
    }

/* (display item) */

static int
display(int args)
    {
    FILE *port = OpenPorts[CurrentOutputIndex];
    scamPP(port,car(args));
    return car(args);
    }

/* (displayAtomic @) */

static int
displayAtomic(int args)
    {
    P_P();
    FILE *port = OpenPorts[CurrentOutputIndex];
    int spot = car(args);
    while (spot != 0)
        {
        scamPP(port,car(spot));
        spot = cdr(spot);
        }
    P_V();
    return 0;
    }


static int
fmt(int args)
    {
    char buffer[512];
    char spec[32];
    int a = car(args);
    int b = cadr(args);

    if (type(a) != STRING)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "fmt given a non-string as format specifier: %s",
            SymbolTable[file(car(args))],line(car(args)),
            type(a)
            );
        }

    if (type(b) == INTEGER)
        snprintf(buffer,sizeof(buffer),cellString(spec,sizeof(spec),a),ival(b));
    else if (type(b) == REAL)
        snprintf(buffer,sizeof(buffer),cellString(spec,sizeof(spec),a),rval(b));
    else
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "fmt given a non-number to be formatted: %s",
            SymbolTable[file(car(args))],line(car(args)),
            type(a));
        }

    return newString(buffer);
    }

/* (pp # item) */

static int
ppp(int args)
    {
    int a = cadr(args);
    FILE *port = OpenPorts[CurrentOutputIndex];

    if (isObject(a))
        ppTable(port,a,0);
    else
        scamPP(port,a);
    return 0;
    }

static int
pppTable(int args)
    {
    int a = car(args);
    FILE *port = OpenPorts[CurrentOutputIndex];

    ppTable(port,a,0);

    return 0;
    }

/* (if # test $then $) */

static int
iif(int args)
    {
    int ret;
    P();
    ENSURE_MEMORY(MAKE_THUNK_SIZE,&args,(int *) 0);

    /* the test expression has already been evaluated */

    if (SameSymbol(cadr(args),FalseSymbol))
        {
        int otherwise = cadddr(args);
        if (otherwise != 0)
            {
            ret = makeThunk(car(otherwise),car(args));
            }
        else
            {
            ret = FalseSymbol;
            }
        }
    else
        {
        ret = makeThunk(caddr(args),car(args));
        }
    V();
    return ret;
    }

/* (cond # $) */

static int
cond(int args)
    {
    int cases = cadr(args);
    int env = car(args);

    PUSH(env); /* peek location will be zero */

    while (cases != 0)
        {
        int result;
        int condition;
        int rule = car(cases);

        if (type(rule) != CONS)
            {
            (void) POP(); /* pop the env */
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "malformed case in cond",
                SymbolTable[file(car(cases))],line(car(cases))
                );
            }

        condition = car(rule);

        PUSH(cases);
        result = eval(condition,env);
        cases = POP();
        
        env = PEEK(0);

        rethrowPop(result,1);

        if (SameSymbol(result,TrueSymbol)) 
            {
            env = POP();
            return evalBlock(cdar(cases),env,ALLBUTLAST);
            }

        cases = cdr(cases);
        }

    (void) POP(); /* pop the env off */

    return FalseSymbol;
    }

/* (while # $test $) */

static int
wwhile(int args)
    {
    int testResult,last;
    PUSH(args); /* peek location will be zero */
    testResult = eval(cadr(args),car(args));
    args = PEEK(0);

    rethrowPop(testResult,1);

    while (SameSymbol(testResult,TrueSymbol))
        {
        last = evalBlock(caddr(args),car(args),ALL);
        args = PEEK(0);
        
        rethrowPop(last,1);

        testResult = eval(cadr(args),car(args));
        args = PEEK(0);

        rethrowPop(testResult,1); /* pop the args */
        }
    (void) POP(); /* pop the args */

    return 0;
    }

/* (set-car! spot value) */

static int
setCar(int args)
    {
    int a = car(args);
    int b = cadr(args);

    char *t = type(a);
    if (t != CONS && t != STRING && t != ARRAY)
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "attempt to set the car of type %s",
            SymbolTable[file(car(args))],line(car(args)),
            t);

    if (t == STRING)
        {
        setcar(a,ival(b));
        }
    else
        {
        setcar(a,b);
        }

    return b;
    }

/* (set-cdr! spot value) */

static int
setCdr(int args)
    {
    if (type(car(args)) != CONS)
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "attempt to set the cdr of type %s",
            SymbolTable[file(car(args))],line(car(args)),
            type(car(args)));

    setcdar(args,cadr(args));
    return cadr(args);
    }

/* (set id value # @) */

static int
set(int args)
    {
    int id = car(args);
    int result;
    
    if (type(id) != SYMBOL)
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "set identifier resolved to type %s, not SYMBOL",
            SymbolTable[file(id)],line(id),
            type(id));

    if (cadddr(args) == 0)
        result = setVariableValue(id,cadr(args),caddr(args));
    else
        result = setVariableValue(id,cadr(args),car(cadddr(args)));
    return result;
    }

/* (get id # @) */

static int 
get(int args)
    {
    int id = car(args);

    if (type(id) != SYMBOL)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "get variable argument resolved to type %s, not SYMBOL",
            SymbolTable[file(args)],line(args),
            type(id));
        }

    if (caddr(args) == 0)
        return getVariableValue(id,cadr(args));
    else
        return getVariableValue(id,car(caddr(args)));
    }

/* (inspect # $item) */

static int
inspect(int args)
    {
    int result;

    PUSH(args);
    result = eval(cadr(args),car(args));
    args = POP();

    if (cadr(args) == 0)
        fprintf(stdout,"nil");
    else
        scamPP(stdout,cadr(args));

    fprintf(stdout," is ");

    if (result == 0)
        fprintf(stdout,"nil");
    else
        scamPP(stdout,result);

    fprintf(stdout,"\n");
    return result;
    }

/* (include # $fileName) */

static int
iinclude(int args)
    {
    int fileName = cadr(args);
    int env = car(args);
    int ptree;
    int s;
    char buffer[512];
    char buffer2[1024];
    PARSER *p;

    PUSH(env); /* peek location will be zilch */

    if (type(fileName) != STRING)
        {
        fileName = eval(fileName,env);
        env = PEEK(0);
        }

    if (type(fileName) != STRING)
        {
        (void) POP(); /* pop the env */
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "include argument was type %s, not STRING",
            SymbolTable[file(args)],line(args),
            type(fileName));
        }

    cellString(buffer,sizeof(buffer),fileName);

    //check to see if file has already been included
    //if so, return false

    sprintf(buffer2,"__included_%s",buffer);

    s = newSymbol(buffer2); /* can cause a gc */
    env = PEEK(0);

    if (findLocal(s,env)) 
        {
        (void) POP(); /* pop the env */
        return FalseSymbol;
        }

    P();
    ENSURE_MEMORY(DEFINE_VARIABLE_SIZE,&s,(int *) 0);

    env = PEEK(0);
    defineVariable(env,s,TrueSymbol);
    V();

    p = newParser(buffer);
    if (p == 0) 
        {
        (void) POP(); // Pop env 
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "file %s could not be opened for reading",
            SymbolTable[file(args)],line(args),
            buffer);
        }

    if (Syntax == SCAM)
        {
        ptree = scamParse(p);
        }
    else
        {
        ptree = swayParse(p);
        }
    fclose(p->input);
    free(p);

    P();
    ENSURE_MEMORY(MAKE_THUNK_SIZE,&ptree,(int *) 0);

    env = POP();

    int o = makeThunk(ptree,env);
    V();

    // Should have env on the stack
    rethrow(ptree);
    
    return o; 
    }

/* (eval expr context) */

static int
eeval(int args)
    {
    P();
    ENSURE_MEMORY(MAKE_THUNK_SIZE,&args,(int *) 0);
    int o = makeThunk(car(args),cadr(args));
    V();
    return o;
    }

/* (apply f args) */

static int
apply(int args)
    {
    P();
    ENSURE_MEMORY(1,&args,(int *) 0);
    int expr = cons(car(args),cadr(args));
    V();
    return evalCall(expr,0,NO_EVALUATION);
    }

static int
ggc(int args)
    {
    if (WorkingThreads == 1)
        {
        P();
        if (GCMode == STOP_AND_COPY)
            {
            stopAndCopy();
            }
        else if (GCMode == MARK_SWEEP)
            {
            reclaim();
            }
        V();
        }
    else
        {
        printf("Implicit calls to GC with multithreading are not supported.");
        }
    return 0;
    }

/* (pass f @) */

static int
pass(int args)
    {
    P();
    ENSURE_MEMORY(1,&args,(int *) 0);
    int expr = cons(car(args),cadr(args));
    V();
    return evalCall(expr,0,PASS_THROUGH);
    }


/* (list? item) */

static int
isList(int args)
    {
    int items = car(args);
    while (type(items) == CONS)
        items = cdr(items);

    return scamBoolean(items == 0);
    }

/* (pair? item) */

static int
isPair(int args)
    {
    return scamBoolean(type(car(args)) == CONS);
    }

/* (null? item) */

static int
isNull(int args)
    {
    return scamBoolean(car(args) == 0);
    }

/* (list @) */

static int
list(int args)
    {
    return car(args);
    }

/* (car item) */

static int
ccar(int args)
    {
    int supply = car(args);
    char *t = type(supply);

    if (t != CONS && t != ARRAY && t != STRING)
        {
        if (supply == 0)
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "attempt to take car of an empty list",
                SymbolTable[file(args)],line(args));
        else
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "attempt to take car of type %s",
                SymbolTable[file(args)],line(args),
                t);
        }

    if (type(supply) == STRING) /* no character type in scam */
        {
        char buffer[2];
        buffer[0] = ival(supply);
        buffer[1] = '\0';
        return newString(buffer);
        }
    else
        return car(supply);
    }

/* (cdr item) */

static int
ccdr(int args)
    {
    int supply = car(args);
    char *t = type(supply);

    if (t != CONS && t != ARRAY && t != STRING)
        {
        if (supply == 0)
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "attempt to take cdr of an empty collection",
                SymbolTable[file(args)],line(args));
        else
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "attempt to take cdr of type %s",
                SymbolTable[file(args)],line(args),
                t);
        }

    return cdr(supply);
    }

/* (cons a b) */

static int
ccons(int args)
    {
    P();
    ENSURE_MEMORY(1,&args,(int *) 0);
    int o = cons(car(args),cadr(args));
    V();
    return o; 
    }

/* (gensym id) */

static int
gensym(int args)
    {
    char buffer[512];
    static int count = 0;

    P();
    if (car(args) == 0)
        snprintf(buffer,sizeof(buffer),"%dsym",count++);
    else
        snprintf(buffer,sizeof(buffer),
            "%d%s",count++,SymbolTable[ival(car(car(args)))]);

    V();
    return newSymbol(buffer);
    }

/* (gensym? id) */

static int
isGensym(int args)
    {
    int id = car(args);

    if (type(id) != SYMBOL) return FalseSymbol;

    return scamBoolean(isdigit(*SymbolTable[ival(id)]));
    }

/*******ports*********************************/

static void
skipWhiteSpace(FILE *fp)
    {
    int ch;
    ch = fgetc(fp);
    while (isspace(ch) && !feof(fp))
        ch = fgetc(fp);
    if (ch != -1)
        ungetc(ch,fp);
    }


static int
addOpenPort(FILE *fp,int portType,int fi,int li,char *fileName)
    {
    int i;
    int maxPorts = sizeof(OpenPorts) / sizeof(FILE *);

    for (i = 3; i < maxPorts; ++i)
        {
        if (OpenPorts[i] == 0)
            break;
        }
    if (i == maxPorts)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "too many ports open at once",
            SymbolTable[fi],li
            );
        }

    OpenPorts[i] = fp;
    OpenPortNames[i] = strdup(fileName);
    if (OpenPortNames[i] == 0)
        Fatal("out of memory\n");
    
    P();
    ENSURE_MEMORY(3,(int *) 0); /* two cons cells plus a new integer */
    int o = cons(portType,cons(newIntegerUnsafe(i),0));
    V();
    return o; 
    }

static int
setPort(int args)
    {
    int old;
    int target;
    int result;

    P();
    ENSURE_MEMORY(3,&args,(int *) 0); /* two cons cells plus a new integer */

    target = car(args);

    if (type(target) == SYMBOL)
        {
        if (ival(target) == stdinIndex)
            {
            old = CurrentInputIndex;
            CurrentInputIndex = 0;
            result = cons(InputPortSymbol,cons(newIntegerUnsafe(old),0));
            V();
            return result;
            }
        else if (ival(target) == stdoutIndex)
            {
            old = CurrentOutputIndex;
            CurrentOutputIndex = 1;
            result = cons(OutputPortSymbol,cons(newIntegerUnsafe(old),0));
            V();
            return result;
            }
        else if (ival(target) == stderrIndex)
            {
            old = CurrentOutputIndex;
            CurrentOutputIndex = 2;
            result = cons(OutputPortSymbol,cons(newIntegerUnsafe(old),0));
            V();
            return result;
            }
        else 
            {
            V();
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "%s is not a valid argument to setPort",
                SymbolTable[file(args)],line(args),
                SymbolTable[ival(target)]);
            }
        }
    else if (type(target) == CONS && SameSymbol(car(target),InputPortSymbol))
        {
        old = CurrentInputIndex;
        CurrentInputIndex = ival(cadr(target));
        int o = consfl(InputPortSymbol,cons(newIntegerUnsafe(old),0),
            file(args),line(args));
        V(); 
        return o;
        }
    else if (type(target) == CONS && SameSymbol(car(target),OutputPortSymbol))
        {
        old = CurrentOutputIndex;
        CurrentOutputIndex = ival(cadr(target));
        int o = consfl(OutputPortSymbol,cons(newIntegerUnsafe(old),0),
            file(args),line(args));
        V();
        return o;
        }

    V();
    return throw(ExceptionSymbol,
        "file %s,line %d: "
        "setPort given a non-port as argument: %s",
        SymbolTable[file(args)],line(args),
        type(target));
    }

static int
getInputPort(int args)
    {
    P();
    ENSURE_MEMORY(3,&args,(int *) 0); /* two cons cells plus a new integer */
    int o = cons(InputPortSymbol,cons(newIntegerUnsafe(CurrentInputIndex),0));
    V();
    return o; 
    }

static int
getOutputPort(int args)
    {
    P();
    ENSURE_MEMORY(3,&args,(int *) 0); /* two cons cells plus a new integer */
    int o = cons(OutputPortSymbol,cons(newIntegerUnsafe(CurrentOutputIndex),0));
    V();
    return o; 
    }

static int
checkValidPort(int index,int fi,int li)
    {
    if (index == 0)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "attempt to close stdin",
            SymbolTable[fi],li
            );
        }
    if (index == 1)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "attempt to close stdout",
            SymbolTable[fi],li
            );
        }
    if (index >= MaxPorts || index < 0)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "attempt to close a non-existent port: %d",
            SymbolTable[fi],li,
            index);
        }
    if (OpenPorts[index] == 0)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "attempt to close an unopened port: %d",
            SymbolTable[fi],li,
            index);
        }

    return 0;
    }

static int
cclose(int args)
    {
    int target,index,newPort;

    target = car(args);

    if (type(target) == CONS)
        {
        if (SameSymbol(car(target),InputPortSymbol))
            newPort = 0;
        else if (SameSymbol(car(target),OutputPortSymbol))
            newPort = 1;
        else
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "close passed a malformed port",
                SymbolTable[file(args)],line(args)
                );

        if (cdr(target) == 0 || type(cadr(target)) != INTEGER)
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "close passed a malformed port",
                SymbolTable[file(args)],line(args)
                );

        index = ival(cadr(target));

        rethrow(checkValidPort(index,file(args),line(args)));
        fclose(OpenPorts[index]);
        OpenPorts[index] = 0;
        free(OpenPortNames[index]);
        OpenPortNames[index] = 0;
        if (CurrentInputIndex == index) CurrentInputIndex = newPort;
        }
    else
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "close passed a malformed port",
            SymbolTable[file(args)],line(args)
            );

    return TrueSymbol;
    }

static int
checkPortForReading(FILE *fp,char *item,int args)
    {
    if (fp == 0)
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "attempt to read %s from a closed port",
            SymbolTable[file(args)],line(args),
            item
            );

    return 0;
    }

static int
readRawChar(int args)
    {
    int ch;
    char buffer[3];
    FILE *fp;

    fp = OpenPorts[CurrentInputIndex];

    rethrow(checkPortForReading(fp,"a raw character",args));

    ch = fgetc(fp);

    if (feof(fp)) return EofSymbol;

    if (ch == '\\')
        {
        ch = fgetc(fp);
        if (ch == 'n')
            buffer[0] = '\n';
        else if (ch == 't')
            buffer[0] = '\t';
        else if (ch == 'r')
            buffer[0] = '\r';
        else
            buffer[0] = ch;
        }
    else
        {
        buffer[0] = ch;
        }

    buffer[1] = '\0';

    return newString(buffer);
    }

static int
readChar(int args)
    {
    char i;
    FILE *fp;
    char buffer[2];

    fp = OpenPorts[CurrentInputIndex];

    rethrow(checkPortForReading(fp,"a character",args));

    i = 0;
    if (fscanf(fp," %c",&i) == EOF) return EofSymbol;
    buffer[0] = i;
    buffer[1] = '\0';
    return newString(buffer);
    }

static int
readInt(int args)
    {
    int i;
    FILE *fp;

    fp = OpenPorts[CurrentInputIndex];

    rethrow(checkPortForReading(fp,"an integer",args));

    i = 0;
    if (fscanf(fp,"%d",&i) == EOF) return EofSymbol;
    return newInteger(i);
    }

static int
readReal(int args)
    {
    double r;
    FILE *fp;

    fp = OpenPorts[CurrentInputIndex];

    rethrow(checkPortForReading(fp,"a real",args));

    r = 0;
    if (fscanf(fp,"%lf",&r) == EOF) return EofSymbol;
    return newReal(r);
    }

static int
readString(int args)
    {
    int ch;
    int index;
    char buffer[4096];
    FILE *fp;

    fp = OpenPorts[CurrentInputIndex];
    rethrow(checkPortForReading(fp,"a string",args));

    skipWhiteSpace(fp);

    ch = fgetc(fp);

    if (feof(fp)) return EofSymbol;

    if (ch != '\"')
        {
        ungetc(ch,fp);
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "found <%c> instead of double quote at the start of a string",
            SymbolTable[file(args)],line(args),
            ch);
        }

    index = 0;
    while ((ch = fgetc(fp)) && ch != EOF)
        {
        if (ch == '\"')
            break;

        if (ch == '\\')
            {
            ch = fgetc(fp);
            if (ch == EOF)
                return throw(ExceptionSymbol,
                    "file %s,line %d: "
                    "attempt to read a string at end of input",
                    SymbolTable[file(args)],line(args)
                    );
            if (ch == 'n')
                buffer[index++] = '\n';
            else if (ch == 't')
                buffer[index++] = '\t';
            else if (ch == 'r')
                buffer[index++] = '\r';
            else
                buffer[index++] = ch;
            }
        else
            {
            buffer[index++] = ch;
            }

        if (index == sizeof(buffer) - 1)
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "attempt to read a very long string failed",
                SymbolTable[file(args)],line(args)
                );
        }

    buffer[index] = '\0';

    if (ch != '\"')
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "attempt to read an unterminated string",
            SymbolTable[file(args)],line(args)
            );

    return newString(buffer);
    }

static int
readExpr(int args)
    {
    int e;
    PARSER *p;
    FILE *fp;

    extern int expr(PARSER *);

    fp = OpenPorts[CurrentInputIndex];

    rethrow(checkPortForReading(fp,"an expression",args));

    p = newParserFP(fp,OpenPortNames[CurrentInputIndex]);

    if (Syntax == SWAY)
        {
        extern int swayInteractive(PARSER *);
        e = swayInteractive(p);
        }
    else
        e = expr(p);

    rethrow(e);

    freeParser(p);

    if (fp == stdin)
        {
        char buffer[8096];
        int index = 0;
        ppToString(e,buffer,sizeof(buffer),&index);
        add_history(buffer);
        }

    return e;
    }

static int
readToken(int args)
    {
    int ch;
    int index;
    char buffer[4096];
    FILE *fp;

    fp = OpenPorts[CurrentInputIndex];

    rethrow(checkPortForReading(fp,"a token",args));

    skipWhiteSpace(fp);

    ch = fgetc(fp);

    if (feof(fp)) return EofSymbol;

    index = 0;
    while (ch != EOF && !isspace(ch))
        {
        buffer[index++] = ch;
        if (index == sizeof(buffer) - 1)
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "attempt to read a very long token failed",
                SymbolTable[file(args)],line(args)
                );
        ch = fgetc(fp);
        }

    buffer[index] = '\0';

    ungetc(ch,fp);

    return newString(buffer);
    }

static int
readWhile(int args)
    {
    int ch;
    int a = car(args);
    int index;
    char target[256];
    char buffer[4096];
    FILE *fp;

    if (type(a) != STRING)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "readWhile argument should be STRING, not type %s",
            SymbolTable[file(args)],line(args),
            type(a));
        }

    fp = OpenPorts[CurrentInputIndex];

    rethrow(checkPortForReading(fp,"characters",args));

    cellStringTr(target,sizeof(target),a);

    ch = fgetc(fp);

    if (feof(fp)) return EofSymbol;

    index = 0;
    while (ch != EOF && strchr(target,ch) != 0)
        {
        buffer[index++] = ch;
        if (index == sizeof(buffer) - 1)
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "attempt to read a very long token failed",
                SymbolTable[file(args)],line(args)
                );
        ch = fgetc(fp);
        }

    buffer[index] = '\0';

    ungetc(ch,fp);

    return newString(buffer);
    }

static int
readUntil(int args)
    {
    int ch;
    int a = car(args);
    int index;
    char target[256];
    char buffer[4096];
    FILE *fp;

    if (type(a) != STRING)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "readWhile argument should be STRING, not type %s",
            SymbolTable[file(args)],line(args),
            type(a));
        }

    fp = OpenPorts[CurrentInputIndex];
    rethrow(checkPortForReading(fp,"characters",args));

    cellStringTr(target,sizeof(target),a);

    ch = fgetc(fp);

    if (feof(fp)) return EofSymbol;

    index = 0;
    while (ch != EOF && strchr(target,ch) == 0)
        {
        buffer[index++] = ch;
        if (index == sizeof(buffer) - 1)
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "attempt to read a very long token failed",
                SymbolTable[file(args)],line(args)
                );
        ch = fgetc(fp);
        }

    buffer[index] = '\0';

    ungetc(ch,fp);

    return newString(buffer);
    }

static int
pushBack(int args)
    {
    int a = car(args);

    if (type(a) == STRING)
        {
        ungetc(ival(a),OpenPorts[CurrentInputIndex]);
        return a;
        }

    return throw(ExceptionSymbol,
        "file %s,line %d: "
        "can only push back a one-character string",
        SymbolTable[file(args)],line(args)
        );
    }

static int
readLine(int args)
    {
    int ch;
    int index;
    char buffer[4096];
    FILE *fp;

    fp = OpenPorts[CurrentInputIndex];

    rethrow(checkPortForReading(fp,"a line",args));
 
    ch = fgetc(fp);

    if (feof(fp)) return EofSymbol;

    index = 0;
    while (ch != EOF && ch != '\n')
        {
        buffer[index++] = ch;
        if (index == sizeof(buffer) - 1)
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "attempt to read a very long line failed",
                SymbolTable[file(args)],line(args)
                );
        ch = fgetc(fp);
        }

    buffer[index] = '\0';

    return newString(buffer);

    }

static int
isEof(int args)
    {
    FILE *fp = OpenPorts[CurrentInputIndex];
    return scamBoolean(fp == 0 || feof(fp));
    }

static int
oopen(int args)
    {
    int target,mode;
    int result;

    target = car(args);
    mode = cadr(args);

    if (type(mode) == SYMBOL)
        {
        if (ival(mode) == readIndex)
            {

            char buffer[512];
            cellString(buffer,sizeof(buffer),target);
            FILE *fp = fopen(buffer,"r");
            
            if (fp == 0)
                return throw(ExceptionSymbol,
                    "file %s,line %d: "
                    "file %s cannot be opened for reading",
                    SymbolTable[file(args)],line(args),
                    buffer);
            
            result = addOpenPort(fp,InputPortSymbol,file(args),line(args),buffer);
            }
        else if (ival(mode) == writeIndex)
            {
            
            char buffer[256];
            cellString(buffer,sizeof(buffer),target);
            FILE *fp = fopen(buffer,"w");
            
            if (fp == 0)
                return throw(ExceptionSymbol,
                    "file %s,line %d: "
                    "file %s cannot be opened for writing",
                    SymbolTable[file(args)],line(args),
                    buffer);
            
            result = addOpenPort(fp,OutputPortSymbol,file(args),line(args),buffer);
            }
        else if (ival(mode) == appendIndex)
            {
            char buffer[256];
            FILE *fp = fopen(cellString(buffer,sizeof(buffer),target),"a");
            if (fp == 0)
                return throw(ExceptionSymbol,
                    "file %s,line %d: "
                    "file %s cannot be opened for appending",
                    SymbolTable[file(args)],line(args),
                    buffer);
            result = addOpenPort(fp,OutputPortSymbol,file(args),line(args),buffer);
            }
        else 
            {
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "%s is an unknown mode "
                "(should be 'read, 'write, or 'append)",
                SymbolTable[file(args)],line(args),
                SymbolTable[ival(mode)]);
            }
        }
    else
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "%s is an unknown mode "
            "(should be 'read, 'write, or 'append)",
            SymbolTable[file(args)],line(args),
            type(mode));
        }

    return result;
    }

static int
ttime(int args)
    {
    struct timeval tv;
    gettimeofday(&tv,(struct timezone *)0);
    return newReal(tv.tv_sec + tv.tv_usec / 1000000.0);
    }

static int
eexit(int args)
    {
    exit(ival(car(args)));
    return 0;
    }

static int
ssystem(int args)
    {
    char buffer[512];
    cellString(buffer,sizeof(buffer),car(args));
    return newInteger(system(buffer));
    }

static int
eexec(int args)
    {
    char buffer[512];
    cellString(buffer,sizeof(buffer),car(args));
    return newInteger(execl("/bin/sh","sh","-c",buffer,(char *) 0));
    }

static int
bindings(int args)
    {
    int items,spot;
    int vars = object_variables(car(args));
    int vals = object_values(car(args));
    int size = length(vars) * 3; /* three conses per variable */

    P();
    ENSURE_MEMORY(size,&vars,&vals,(int *) 0);

    items = 0;
    /* ignore mismatches between vars and vals */
    while (vars != 0 && vals != 0)
        {
        int pack = cons(cons(car(vars),cons(car(vals),0)),0);

        if (items == 0)
            {
            items = pack;
            spot = items;
            }
        else
            {
            setcdr(spot,pack);
            spot = cdr(spot);
            }
        vars = cdr(vars);
        vals = cdr(vals);
        }
    V();
    return items;
    }

/* (array @) */

static int
array(int args)
    {
    int size,start,spot;

    args = car(args);
    size = length(args);

    P();
    ENSURE_CONTIGUOUS_MEMORY(size,&args,(int *) 0);

    start = allocateContiguous(ARRAY,size);

    for (spot = start; spot < start + size; ++spot)
        {
        setcar(spot,car(args));
        args = cdr(args);
        }
    V();
    return start;
    }

/* (allocate size) */

static int
allocate(int args)
    {
    int size = ival(car(args));

    P();
    ENSURE_CONTIGUOUS_MEMORY(size,(int *) 0);

    int result = allocateContiguous(ARRAY,size); /* sets cars to zero */
    V();
    return result;
    }

/* (length item) */

static int
llength(int args)
    {
    int item = car(args);

    if (item == 0)
        return newInteger(0);
    else if (type(item) == ARRAY)
        return newInteger(count(item));
    else if (type(item) == STRING)
        return newInteger(count(item));
    else if (type(item) == CONS)
        {
        int size = 0;
        while (type(item) == CONS)
            {
            ++size;
            item = cdr(item);
            }
        return newInteger(size);
        }
    else if (type(item) == SYMBOL)
        return newInteger(strlen(SymbolTable[ival(car(args))]));
    else
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "cannot take the length of type %s",
            SymbolTable[file(args)],line(args),
            type(item));
    }

/* (getElement array index) */

static int
getElement(int args)
    {
    int limit;
    int supply = car(args);
    int index = ival(cadr(args));
    char *kind = type(supply);

    if (index < 0)
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "negative indices (%d) are not allowed",
            SymbolTable[file(args)],line(args),
            index);

    if (kind == ARRAY)
        limit = count(supply);
    else if (kind == STRING || kind == CONS)
        limit = length(supply);
    else
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "cannot index into type %s",
            SymbolTable[file(args)],line(args),
            kind);

    if (index >= limit)
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "index (%d) is too large",
            SymbolTable[file(args)],line(args),
            index);

    if (type(supply) == ARRAY)
        {
        int result;
        result = car(supply + index);
        return result;
        }
    else
        {
        while (index > 0)
            {
            supply = cdr(supply);
            --index;
            }
        if (type(supply) == STRING)
            {
            char buffer[2];
            buffer[0] = ival(supply);
            buffer[1] = '\0';
            return newString(buffer);
            }
        else
            return car(supply);
        }
    }

/* (setElement! array index value) */

static int
setElement(int args)
    {
    int limit;
    int supply = car(args);
    int index = ival(cadr(args));
    int value = caddr(args);
    char *kind = type(supply);

    if (index < 0)
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "negative indices (%d) are not allowed",
            SymbolTable[file(args)],line(args),
            index);

    if (kind == ARRAY)
        limit = count(supply);
    else if (kind == STRING || kind == CONS)
        limit = length(supply);
    else
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "cannot index into type %s",
            SymbolTable[file(args)],line(args),
            kind);

    if (index >= limit)
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "index (%d) is too large",
            SymbolTable[file(args)],line(args),
            index);

    if (type(supply) == ARRAY || type(supply) == STRING)
        {
        setcar(supply + index,value);
        return car(supply + index);
        }
    else
        {
        while (index > 0)
            {
            supply = cdr(supply);
            --index;
            }

        if (type(supply) == CONS)
            {
            setcar(supply,value);
            }
        else
            {
            setcar(supply,ival(value));
            }
        return value;
        }
    }

static int
symbol(int args)
    {
    char buffer[1024];
    cellString(buffer,sizeof(buffer),car(args));
    return newSymbol(buffer);
    }

/* (catch # $expr) */

static int
catch(int args)
    {
    int result;

    result = eval(cadr(args),car(args));

    if (isThrow(result))
        {
        set_object_label(result,ErrorSymbol);
        }

    return result;
    }

/* (throw code @) */

static int
tthrow(int args)
    {
    int item;
    int o;

    P();
    ENSURE_MEMORY(MAKE_THROW_SIZE,&args,(int *) 0);

    item = car(args);

    if (cadr(args) == 0 && isError(item))
        {
        o = makeThrow(error_code(item),error_value(item),error_trace(item));
        V();
        return o; 
        }
    else if (cadr(args) != 0)
        {
        o = makeThrow(item,car(cadr(args)),0);
        V();
        return o; 
        }
    else
        {
        V();
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "wrong number of arguments to throw",
            SymbolTable[file(args)],line(args)
            );
        }
    }

/* string manipulations */

/* (string-append a b) */

static int
string_append(int args)
    {
    int a = car(args);
    int b = cadr(args);
    int spot,sizeA,sizeB,size,start;

    if (a != 0 && type(a) != STRING)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "prefix: first argument should be STRING, not %s",
            SymbolTable[file(args)],line(args),
            type(a));
        }

    if (b != 0 && type(b) != STRING)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "prefix: first argument should be STRING, not %s",
            SymbolTable[file(args)],line(args),
            type(b));
        }

    if (a == 0 && b == 0) return 0;

    sizeA = length(a);
    sizeB = length(b);
    size = sizeA + sizeB;

    P();
    ENSURE_CONTIGUOUS_MEMORY(size,&a,&b,(int *) 0);
    start = allocateContiguous(STRING,size);
    V();

    for (spot = start; spot < start + size; ++spot)
        {
        /* if there are characters left in a, append from a first */
        if (spot - start < sizeA)
            {
            setcar(spot,car(a));
            a = cdr(a);
            }
        else
            {
            setcar(spot,car(b));
            b = cdr(b);
            }
        }
    
    return start;
    }

/* (string-equal a b) */

static int
string_equal(int args)
    {
    int a = car(args);
    int b = cadr(args);

    while (a != 0)
        {
        if (b == 0) return FalseSymbol;
        if (ival(a) != ival(b)) return FalseSymbol;
        a = cdr(a);
        b = cdr(b);
        }

    return scamBoolean(b == 0);
    }

/* (prefix str size) */

static int
prefix(int args)
    {
    int i,count;
    int a = car(args);
    int b = cadr(args);
    int result;
    char *buffer;

    if (type(a) != STRING)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "prefix: first argument should be STRING, not %s",
            SymbolTable[file(args)],line(args),
            type(a));
        }

    if (type(b) != INTEGER)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "prefix: second argument should be INTEGER, not %s",
            SymbolTable[file(args)],line(args),
            type(b));
        }

    buffer = (char *) malloc(ival(b) + 1);
    if(buffer == NULL)
    {
        return throw(ExceptionSymbol,
                "file %s, line %d: "
                "prefix: Out of memory for",
                SymbolTable[file(args)],line(args)
                );
    }

    count = 0;
    for (i = 0; ival(a) != '\0' && i < ival(b); ++i)
        {
        buffer[i] = ival(a);
        a = cdr(a);
        ++count;
        }

    buffer[count] = '\0';

    result = newString(buffer);
    free(buffer);

    return result;
    }

static int
suffix(int args)
    {
    int a = car(args);
    int b = cadr(args);

    if (type(a) != STRING)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "suffix: first argument should be STRING, not %s",
            SymbolTable[file(args)],line(args),
            type(a));
        }

    if (type(b) != INTEGER)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "suffix: second argument should be INTEGER, not %s",
            SymbolTable[file(args)],line(args),
            type(b));
        }


    if (count(a) < ival(b)) return newString("");

    return a + ival(b);
    }

static int
stringWhile(int args)
    {
    int ch;
    int a = car(args);
    int b = cadr(args);
    int count;
    char target[256];

    if (type(a) != STRING || type(b) != STRING)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "stringWhile: both arguments should be STRING, not %s and %s",
            SymbolTable[file(args)],line(args),
            type(a),type(b));
        }

    cellStringTr(target,sizeof(target),b);

    count = 0;
    while ((ch = ival(a)) && ch != '\0' && strchr(target,ch) != 0)
        {
        a = cdr(a);
        ++count;
        }

    return newInteger(count);
    }

static int
stringUntil(int args)
    {
    int ch;
    int a = car(args);
    int b = cadr(args);
    int count;
    char target[256];

    if (type(a) != STRING || type(b) != STRING)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "stringUntil: both arguments should be STRING, not %s and %s",
            SymbolTable[file(args)],line(args),
            type(a),type(b));
        }

    cellStringTr(target,sizeof(target),b);

    count = 0;
    while ((ch = ival(a)) && ch != '\0' && strchr(target,ch) == 0)
        {
        ++count;
        a = cdr(a);
        }

    return newInteger(count);
    }

static int
substring(int args)
    {
    int needle = car(args);
    int haystack = cadr(args);

    if (type(needle) != STRING)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "substring: first argument is type %s (should be %s)",
            SymbolTable[file(args)],line(args),
            type(needle),STRING);
        }

    if (type(haystack) != STRING)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "substring: second argument is %s (should be %s)",
            SymbolTable[file(args)],line(args),
            type(haystack),STRING);
        }

    while (haystack != 0)
        {
        int n = needle;
        int h = haystack;
        while (ival(n) != 0)
            {
            if (ival(n) != ival(h))
                break;
            n = cdr(n);
            h = cdr(h);
            }
        if (ival(n) == 0) return haystack;
        haystack = cdr(haystack);
        }

    return 0;
    }

static int
trim(int args)
    {
    int a = car(args);
    int start,length;

    if (type(a) != STRING)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "trim: argument is type %s (should be %s)",
            SymbolTable[file(args)],line(args),
            type(a),STRING);
        }

    while (isspace(ival(a)))
        a = cdr(a);

    length = count(a) - 1;

    while (isspace(ival(a + length)))
        --length;

   
    P();
    ENSURE_CONTIGUOUS_MEMORY(length,&a,(int *) 0);
    start = allocateContiguous(STRING,length); /* allocate 'length' cells */
    V();

    int spot = start;
    while (spot != 0)
        {
        setival(spot,ival(a));
        spot = cdr(spot);
        a = cdr(a);
        }

    return start;
    }
        
static int
ascii(int args)
    {
    int a = car(args);

    if (type(a) != STRING)
        {
        return throw(ExceptionSymbol,
            "file %s,line %d: "
            "ascii: argument is type %s (should be %s)",
            SymbolTable[file(args)],line(args),
            type(a),STRING);
        }

    return newInteger(ival(a));
    }
        
/* (ord char) */

static int
ord(int args)
    {
    return newInteger(ival(car(args)));
    }

/* (char int) */

static int
cchar(int args)
    {
    char buffer[2];
    buffer[0] = ival(car(args));
    buffer[1] = '\0';
    return newString(buffer);
    }

static int
iint(int args)
    {
    int a = car(args);
    char buffer[128];

    if (type(a) == INTEGER) return a;
    
    if (type(a) == REAL) return newInteger((int) rval(a));

    if (type(a) == STRING)
        {
        cellString(buffer,sizeof(buffer),a);
        return newInteger(atoi(buffer));
        }

    return throw(ExceptionSymbol,
        "file %s,line %d: "
        "int: argument is type %s (should be %s or %s)",
        SymbolTable[file(args)],line(args),
        type(a),REAL,STRING);
    }
        
static int
rreal(int args)
    {
    int a = car(args);
    char buffer[128];

    if (type(a) == REAL) return a;
    
    if (type(a) == INTEGER) return newReal((double) ival(a));

    if (type(a) == STRING)
        {
        cellString(buffer,sizeof(buffer),a);
        return newReal(atof(buffer));
        }

    return throw(ExceptionSymbol,
        "file %s,line %d: "
        "int: argument is type %s (should be %s or %s)",
        SymbolTable[file(args)],line(args),
        type(a),INTEGER,STRING);
    }

static int
sstring(int args)
    {
    int a = car(args);
    char buffer[1024];

    if (type(a) == STRING) return a;
    
    if (type(a) == INTEGER)
        {
        snprintf(buffer,sizeof(buffer),"%d",ival(a));
        return newString(buffer);
        }

    if (type(a) == SYMBOL)
        {
        snprintf(buffer,sizeof(buffer),"%s",SymbolTable[ival(a)]);
        return newString(buffer);
        }

    if (type(a) == REAL)
        {
        snprintf(buffer,sizeof(buffer),"%f",rval(a));
        return newString(buffer);
        }

    return throw(ExceptionSymbol,
        "file %s,line %d: "
        "int: argument is type %s (should be %s or %s)",
        SymbolTable[file(args)],line(args),
        type(a),REAL,STRING);
    }
        

/* (address item) */

static int
address(int args)
    {
    return newInteger(car(args));
    }

/* (setCaching mode) */

static int
setCaching(int args)
    {
    int old = Caching;

    if (SameSymbol(car(args),TrueSymbol))
        Caching = 1;
    else
        Caching = 0;

    return old == 0? FalseSymbol : TrueSymbol;
    }

/* (getCaching) */

static int
getCaching(int args)
    {
    return Caching == 0? FalseSymbol : TrueSymbol;
    }

/* (gcCount) */

static int
ggcCount(int args)
    {
    return newInteger(GCCount);
    }

/* (line item) */

static int
lline(int args)
    {
    return newInteger(line(car(args)));
    }

/* (file item) */

static int
ffile(int args)
    {
    char buffer[512];
    snprintf(buffer,sizeof(buffer),"%s",filename(car(args)));
    return newString(buffer);
    }


/* (sleep seconds)
 * Note: this is a high precision sleep using nanosleep.  
 * 1 second is 1000000000 nanoseconds
 */
int
ssleep(int args)
    {
    struct timespec tim;
    int val = ival(car(args));
    tim.tv_nsec = val % 1000000000;
    tim.tv_sec = (val / 1000000000);
    nanosleep(&tim,NULL);
    return 0;
    }

void
loadBuiltIns(int env)
    {
    int b;
    int count = 0;

    BuiltIns[count] = getTID;
    b = makeBuiltIn(env,
        newSymbolUnsafe("gettid"),
        0,
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ssleep;
    b = makeBuiltIn(env,
        newSymbolUnsafe("sleep"),
        cons(newSymbolUnsafe("seconds"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = string_append;
    b = makeBuiltIn(env,
        newSymbolUnsafe("string-append"),
        cons(newSymbolUnsafe("a"),
            cons(newSymbolUnsafe("b"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = string_equal;
    b = makeBuiltIn(env,
        newSymbolUnsafe("string-equal?"),
        cons(newSymbolUnsafe("a"),
            cons(newSymbolUnsafe("b"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = prefix;
    b = makeBuiltIn(env,
        newSymbolUnsafe("prefix"),
        cons(newSymbolUnsafe("str"),
            cons(newSymbolUnsafe("size"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = suffix;
    b = makeBuiltIn(env,
        newSymbolUnsafe("suffix"),
        cons(newSymbolUnsafe("str"),
            cons(newSymbolUnsafe("size"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = stringUntil;
    b = makeBuiltIn(env,
        newSymbolUnsafe("stringUntil"),
        cons(newSymbolUnsafe("expr"),
            cons(newSymbolUnsafe("chars"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = stringWhile;
    b = makeBuiltIn(env,
        newSymbolUnsafe("stringWhile"),
        cons(newSymbolUnsafe("str"),
            cons(newSymbolUnsafe("chars"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = substring;
    b = makeBuiltIn(env,
        newSymbolUnsafe("substring"),
        cons(newSymbolUnsafe("needle"),
            cons(newSymbolUnsafe("haystack"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = trim;
    b = makeBuiltIn(env,
        newSymbolUnsafe("trim"),
        cons(newSymbolUnsafe("str"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ascii;
    b = makeBuiltIn(env,
        newSymbolUnsafe("ascii"),
        cons(newSymbolUnsafe("str"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ord;
    b = makeBuiltIn(env,
        newSymbolUnsafe("ord"),
        cons(newSymbolUnsafe("char"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = cchar;
    b = makeBuiltIn(env,
        newSymbolUnsafe("char"),
        cons(newSymbolUnsafe("int"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = iint;
    b = makeBuiltIn(env,
        newSymbolUnsafe("int"),
        cons(newSymbolUnsafe("item"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = rreal;
    b = makeBuiltIn(env,
        newSymbolUnsafe("real"),
        cons(newSymbolUnsafe("item"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = sstring;
    b = makeBuiltIn(env,
        newSymbolUnsafe("string"),
        cons(newSymbolUnsafe("item"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = catch;
    b = makeBuiltIn(env,
        newSymbolUnsafe("catch"),
        cons(SharpSymbol,
            cons(newSymbolUnsafe("$expr"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = tthrow;
    b = makeBuiltIn(env,
        newSymbolUnsafe("throw"),
        cons(newSymbolUnsafe("code"),
            cons(AtSymbol,0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = symbol;
    b = makeBuiltIn(env,
        newSymbolUnsafe("symbol"),
        cons(newSymbolUnsafe("str"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = getElement;
    b = makeBuiltIn(env,
        newSymbolUnsafe("getElement"),
        cons(newSymbolUnsafe("item"),
            cons(newSymbolUnsafe("index"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = setElement;
    b = makeBuiltIn(env,
        newSymbolUnsafe("setElement"),
        cons(newSymbolUnsafe("item"),
            cons(newSymbolUnsafe("index"),
                cons(newSymbolUnsafe("value"),0))),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = llength;
    b = makeBuiltIn(env,
        newSymbolUnsafe("length"),
        cons(newSymbolUnsafe("item"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = allocate;
    b = makeBuiltIn(env,
        newSymbolUnsafe("allocate"),
        cons(newSymbolUnsafe("size"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = array;
    b = makeBuiltIn(env,
        newSymbolUnsafe("array"),
        cons(AtSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = bindings;
    b = makeBuiltIn(env,
        newSymbolUnsafe("bindings"),
        cons(newSymbolUnsafe("object"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = eexit;
    b = makeBuiltIn(env,
        newSymbolUnsafe("exit"),
        cons(newSymbolUnsafe("errorNumber"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ssystem;
    b = makeBuiltIn(env,
        newSymbolUnsafe("system"),
        cons(newSymbolUnsafe("str"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = eexec;
    b = makeBuiltIn(env,
        newSymbolUnsafe("exec"),
        cons(newSymbolUnsafe("str"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ttime;
    b = makeBuiltIn(env,
        newSymbolUnsafe("time"),
        0,
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readChar;
    b = makeBuiltIn(env,
        newSymbolUnsafe("readChar"),
        cons(SharpSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readRawChar;
    b = makeBuiltIn(env,
        newSymbolUnsafe("readRawChar"),
        cons(SharpSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readInt;
    b = makeBuiltIn(env,
        newSymbolUnsafe("readInt"),
        cons(SharpSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readReal;
    b = makeBuiltIn(env,
        newSymbolUnsafe("readReal"),
        cons(SharpSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readString;
    b = makeBuiltIn(env,
        newSymbolUnsafe("readString"),
        cons(SharpSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readToken;
    b = makeBuiltIn(env,
        newSymbolUnsafe("readToken"),
        cons(SharpSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readExpr;
    b = makeBuiltIn(env,
        newSymbolUnsafe("readExpr"),
        cons(SharpSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readLine;
    b = makeBuiltIn(env,
        newSymbolUnsafe("readLine"),
        cons(SharpSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readWhile;
    b = makeBuiltIn(env,
        newSymbolUnsafe("readWhile"),
        cons(newSymbolUnsafe("string"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readUntil;
    b = makeBuiltIn(env,
        newSymbolUnsafe("readUntil"),
        cons(newSymbolUnsafe("string"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = pushBack;
    b = makeBuiltIn(env,
        newSymbolUnsafe("pushBack"),
        cons(newSymbolUnsafe("string"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = oopen;
    b = makeBuiltIn(env,
        newSymbolUnsafe("open"),
        cons(newSymbolUnsafe("name"),
            cons(newSymbolUnsafe("mode"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = setPort;
    b = makeBuiltIn(env,
        newSymbolUnsafe("setPort"),
        cons(newSymbolUnsafe("port"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = getOutputPort;
    b = makeBuiltIn(env,
        newSymbolUnsafe("getOutputPort"),
        cons(SharpSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = getInputPort;
    b = makeBuiltIn(env,
        newSymbolUnsafe("getInputPort"),
        cons(SharpSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = cclose;
    b = makeBuiltIn(env,
        newSymbolUnsafe("close"),
        cons(newSymbolUnsafe("port"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isEof;
    b = makeBuiltIn(env,
        newSymbolUnsafe("eof?"),
        cons(SharpSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = gensym;
    b = makeBuiltIn(env,
        newSymbolUnsafe("gensym"),
        cons(AtSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isGensym;
    b = makeBuiltIn(env,
        newSymbolUnsafe("gensym?"),
        cons(newSymbolUnsafe("id"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ccons;
    b = makeBuiltIn(env,
        newSymbolUnsafe("cons"),
        cons(newSymbolUnsafe("a"),
            cons(newSymbolUnsafe("b"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ccdr;
    b = makeBuiltIn(env,
        newSymbolUnsafe("cdr"),
        cons(newSymbolUnsafe("items"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ccar;
    b = makeBuiltIn(env,
        newSymbolUnsafe("car"),
        cons(newSymbolUnsafe("items"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ttype;
    b = makeBuiltIn(env,
        newSymbolUnsafe("type"),
        cons(newSymbolUnsafe("item"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isNull;
    b = makeBuiltIn(env,
        newSymbolUnsafe("null?"),
        cons(newSymbolUnsafe("item"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isPair;
    b = makeBuiltIn(env,
        newSymbolUnsafe("pair?"),
        cons(newSymbolUnsafe("item"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isList;
    b = makeBuiltIn(env,
        newSymbolUnsafe("list?"),
        cons(newSymbolUnsafe("item"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = list;
    b = makeBuiltIn(env,
        newSymbolUnsafe("list"),
        cons(AtSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = iinclude;
    b = makeBuiltIn(env,
        newSymbolUnsafe("include"),
        cons(SharpSymbol,
            cons(newSymbolUnsafe("$fileName"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = eeval;
    b = makeBuiltIn(env,
        newSymbolUnsafe("eval"),
        cons(newSymbolUnsafe("expr"),
            cons(newSymbolUnsafe("context"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = apply;
    b = makeBuiltIn(env,
        newSymbolUnsafe("apply"),
        cons(newSymbolUnsafe("f"),
            cons(newSymbolUnsafe("args"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = pass;
    b = makeBuiltIn(env,
        newSymbolUnsafe("pass"),
        cons(newSymbolUnsafe("f"),
            cons(newSymbolUnsafe("@"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = inspect;
    b = makeBuiltIn(env,
        newSymbolUnsafe("inspect"),
        cons(SharpSymbol,
            cons(newSymbolUnsafe("$expr"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = scope;
    b = makeBuiltIn(env,
        newSymbolUnsafe("scope"),
        cons(SharpSymbol,
            cons(DollarSymbol,0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = begin;
    b = makeBuiltIn(env,
        BeginSymbol,
        cons(SharpSymbol,
            cons(DollarSymbol,0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = rreturn;
    b = makeBuiltIn(env,
        ReturnSymbol,
        cons(SharpSymbol,
            cons(newSymbolUnsafe("$item"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = define;
    b = makeBuiltIn(env,
        newSymbolUnsafe("define"),
        cons(SharpSymbol,
            cons(DollarSymbol,0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = define;
    b = makeBuiltIn(env,
        newSymbolUnsafe("__define"),
        cons(newSymbolUnsafe("env"),
            cons(newSymbolUnsafe("rest"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = addSymbol;
    b = makeBuiltIn(env,
        newSymbolUnsafe("addSymbol"),
        cons(newSymbolUnsafe("name"),
            cons(newSymbolUnsafe("init"),
                cons(newSymbolUnsafe("env"),0))),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = display;
    b = makeBuiltIn(env,
        newSymbolUnsafe("display"),
        cons(newSymbolUnsafe("item"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = displayAtomic;
    b = makeBuiltIn(env,
        newSymbolUnsafe("displayAtomic"),
        cons(AtSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = fmt;
    b = makeBuiltIn(env,
        newSymbolUnsafe("fmt"),
        cons(newSymbolUnsafe("format"),
            cons(ValueSymbol,0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ppp;
    b = makeBuiltIn(env,
        newSymbolUnsafe("pp"),
        cons(SharpSymbol,
            cons(newSymbolUnsafe("a"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = pppTable;
    b = makeBuiltIn(env,
        newSymbolUnsafe("ppTable"),
        cons(newSymbolUnsafe("a"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = plus;
    b = makeBuiltIn(env,
        newSymbolUnsafe("+"),
        cons(AtSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = minus;
    b = makeBuiltIn(env,
        newSymbolUnsafe("-"),
        cons(AtSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = times;
    b = makeBuiltIn(env,
        newSymbolUnsafe("*"),
        cons(AtSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = divides;
    b = makeBuiltIn(env,
        newSymbolUnsafe("/"),
        cons(AtSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = mod;
    b = makeBuiltIn(env,
        newSymbolUnsafe("%"),
        cons(AtSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = eexp;
    b = makeBuiltIn(env,
        newSymbolUnsafe("exp"),
        cons(newSymbolUnsafe("n"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = llog;
    b = makeBuiltIn(env,
        newSymbolUnsafe("log"),
        cons(newSymbolUnsafe("n"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ssin;
    b = makeBuiltIn(env,
        newSymbolUnsafe("sin"),
        cons(newSymbolUnsafe("n"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = coss;
    b = makeBuiltIn(env,
        newSymbolUnsafe("cos"),
        cons(newSymbolUnsafe("n"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = aatan;
    b = makeBuiltIn(env,
        newSymbolUnsafe("atan"),
        cons(newSymbolUnsafe("y"),
            cons(newSymbolUnsafe("x"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = expt;
    b = makeBuiltIn(env,
        newSymbolUnsafe("expt"),
        cons(newSymbolUnsafe("base"),
            cons(newSymbolUnsafe("exponent"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = randomInt;
    b = makeBuiltIn(env,
        newSymbolUnsafe("randomInt"),
        0,
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = randomMax;
    b = makeBuiltIn(env,
        newSymbolUnsafe("randomMax"),
        0,
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = randomSeed;
    b = makeBuiltIn(env,
        newSymbolUnsafe("randomSeed"),
        cons(newSymbolUnsafe("seed"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = quote;
    b = makeBuiltIn(env,
        newSymbolUnsafe("quote"),
        cons(newSymbolUnsafe("$item"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = lambda;
    b = makeBuiltIn(env,
        newSymbolUnsafe("lambda"),
        cons(SharpSymbol,
            cons(newSymbolUnsafe("$params"),
                cons(DollarSymbol,0))),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = closure;
    b = makeBuiltIn(env,
        newSymbolUnsafe("closure"),
        cons(SharpSymbol,
            cons(newSymbolUnsafe("name"),
                cons(newSymbolUnsafe("params"),
                    cons(newSymbolUnsafe("body"),
                        cons(newSymbolUnsafe("env"),0))))),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = cond;
    b = makeBuiltIn(env,
        newSymbolUnsafe("cond"),
        cons(SharpSymbol,
            cons(DollarSymbol,0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = iif;
    b = makeBuiltIn(env,
        newSymbolUnsafe("if"),
        cons(SharpSymbol,
            cons(newSymbolUnsafe("test"),
                cons(newSymbolUnsafe("$then"),
                    cons(DollarSymbol,0)))),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = wwhile;
    b = makeBuiltIn(env,
        newSymbolUnsafe("while"),
        cons(SharpSymbol,
            cons(newSymbolUnsafe("$test"),
                cons(DollarSymbol,0))),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = address;
    b = makeBuiltIn(env,
        newSymbolUnsafe("address"),
        cons(newSymbolUnsafe("item"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = setCaching;
    b = makeBuiltIn(env,
        newSymbolUnsafe("setCaching"),
        cons(newSymbolUnsafe("mode"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = getCaching;
    b = makeBuiltIn(env,
        newSymbolUnsafe("getCaching"),
        0,
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ggc;
    b = makeBuiltIn(env,
        newSymbolUnsafe("gc"),
        0,
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = thread;
    b = makeBuiltIn(env,
        newSymbolUnsafe("thread"),
        cons(SharpSymbol,
            cons(newSymbolUnsafe("$expr"),0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = lock;
    b = makeBuiltIn(env,
        newSymbolUnsafe("lock"),
        0,
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = unlock;
    b = makeBuiltIn(env,
        newSymbolUnsafe("unlock"),
        0,
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;


    BuiltIns[count] = tjoin;
    b = makeBuiltIn(env,
        newSymbolUnsafe("tjoin"),
        cons(newSymbolUnsafe("tid"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = not;
    b = makeBuiltIn(env,
        newSymbolUnsafe("not"),
        cons(ValueSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isLessThan;
    b = makeBuiltIn(env,
        newSymbolUnsafe("<"),
        cons(AtSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isLessThanOrEqualTo;
    b = makeBuiltIn(env,
        newSymbolUnsafe("<="),
        cons(AtSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isGreaterThan;
    b = makeBuiltIn(env,
        newSymbolUnsafe(">"),
        cons(AtSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isGreaterThanOrEqualTo;
    b = makeBuiltIn(env,
        newSymbolUnsafe(">="),
        cons(AtSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isEq;
    b = makeBuiltIn(env,
        newSymbolUnsafe("eq?"),
        cons(AtSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isEq;
    b = makeBuiltIn(env,
        newSymbolUnsafe("=="),
        cons(AtSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isNotEq;
    b = makeBuiltIn(env,
        newSymbolUnsafe("neq?"),
        cons(AtSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isNumericEqualTo;
    b = makeBuiltIn(env,
        newSymbolUnsafe("="),
        cons(AtSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isNotEq;
    b = makeBuiltIn(env,
        newSymbolUnsafe("!="),
        cons(AtSymbol,0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = get;
    b = makeBuiltIn(env,
        newSymbolUnsafe("get"),
        cons(newSymbolUnsafe("id"),
            cons(SharpSymbol,
                cons(AtSymbol,0))),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = set;
    b = makeBuiltIn(env,
        newSymbolUnsafe("set"),
        cons(newSymbolUnsafe("id"),
            cons(ValueSymbol,
                cons(SharpSymbol,
                    cons(AtSymbol,0)))),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = setCar;
    b = makeBuiltIn(env,
        newSymbolUnsafe("set-car!"),
        cons(newSymbolUnsafe("spot"),
            cons(ValueSymbol,0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = setCdr;
    b = makeBuiltIn(env,
        newSymbolUnsafe("set-cdr!"),
        cons(newSymbolUnsafe("spot"),
            cons(ValueSymbol,0)),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ggcCount;
    b = makeBuiltIn(env,
        newSymbolUnsafe("gcCount"),
        0,
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = lline;
    b = makeBuiltIn(env,
        newSymbolUnsafe("lineNumber"),
        cons(newSymbolUnsafe("item"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ffile;
    b = makeBuiltIn(env,
        newSymbolUnsafe("fileName"),
        cons(newSymbolUnsafe("item"),0),
        newIntegerUnsafe(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    int allocated = HeapBottom +
        count * (3 + MAKE_BUILTIN_SIZE + DEFINE_VARIABLE_SIZE);

    if (allocated >= MemorySize)
        {
        Fatal("Memory Size is WAAAAAY too small!\n");
        }

    assert(count <= sizeof(BuiltIns) / sizeof(PRIM));

    OpenPorts[0] = stdin;
    OpenPorts[1] = stdout;
    OpenPorts[2] = stderr;

    OpenPortNames[0] = strdup("stdin");
    OpenPortNames[1] = strdup("stdout");
    OpenPortNames[2] = strdup("stderr");

    CurrentInputIndex = 0;
    CurrentOutputIndex = 1;
    }

void
installArgsEnv(int argIndex,int argc, char **argv, char **envv, int env)
    {
    int index;
    int start;
    int spot;

    index = 0;
    if (argc - argIndex > 0)
        {
        start = allocateContiguous(ARRAY,argc-argIndex);
        spot = start;

        while (spot != 0)
            {
            int o = newStringUnsafe(argv[argIndex]);
            setcar(spot,o);
            ++argIndex;
            spot = cdr(spot);
            }

        defineVariable(env,newSymbolUnsafe(ArgumentsName),start);
        }
    else
        {
        defineVariable(env,newSymbolUnsafe(ArgumentsName),0);
        }

    argc = 0;
    while (envv[argc] != 0)
        {
        if (strstr(envv[argc],LibraryName) == envv[argc])
            {
            LibraryPointer = strdup(strchr(envv[argc],'=') + 1);
            }
        ++argc;
        }

    if (argc > 0)
        {
        start = allocateContiguous(ARRAY,argc);
        spot = start;

        index = 0;
        while (spot != 0)
            {
            int o = newStringUnsafe(envv[index]);
            setcar(spot,o); 
            spot = cdr(spot);
            ++index;
            }

        defineVariable(env,newSymbolUnsafe(EnvironmentName),start);
        }
    else
        defineVariable(env,newSymbolUnsafe(EnvironmentName),0);
    }
