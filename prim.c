#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "types.h"
#include "env.h"
#include "cell.h"
#include "parser.h"
#include "prim.h"
#include "eval.h"
#include "pp.h"
#include "util.h"

extern char *LibraryName;
extern char *LibraryPointer;
extern char *ArgumentsName;
extern char *EnvironmentName;

extern void gc(void);

PRIM BuiltIns[1000];
FILE *OpenPorts[20];
int MaxPorts = sizeof(OpenPorts) / sizeof(FILE *);
int CurrentInputIndex;
int CurrentOutputIndex;

static int
scamBoolean(int item)
    {
    if (item == 0)
        return falseSymbol;
    else
        return trueSymbol;
    }

/* (gc) */

static int
ggc()
    {
    gc();
    return 0;
    }

/* (quote $item) */

static int
quote(int args)
    {
    return car(args);
    }

static int
defineIdentifier(int name,int init,int env)
    {
    //debug("init",init);
    if (init != 0 && !sameSymbol(init,uninitializedSymbol))
        {
        push(name);
        push(env);
        init = eval(init,env);
        env = pop();
        name = pop();

        rethrow(init,0);
        }

     assureMemory("defineIdentifier",DEFINE_CELLS,&env,&name,&init,0);

     return defineVariable(env,name,init);
     }
        
static int
defineFunction(int name,int parameters,int body,int env)
    {
    int spot;
    int closure;
    
    //printf("defining a function...\n");
    //debug("    name:",name);
    //debug("    params:",parameters);
    //debug("    body:",body);
    assureMemory("defineFunction",CLOSURE_CELLS + DEFINE_CELLS,&name,&parameters,&body,&env,0);

    spot = parameters;
    while (spot != 0)
        {
        int sym = car(spot);
        if (type(sym) != SYMBOL)
            {
            return throw(exceptionSymbol,
                "file %s,line %d: "
                "only SYMBOLS can be parameters, found type %s",
                SymbolTable[file(name)],line(name),
                type(sym));
            }
        spot = cdr(spot);
        }

    closure = makeClosure(env,name,parameters,body,ADD_BEGIN);

    return defineVariable(env,name,closure);
    }

/* (define # $) */

static int
define(int args)
    {
    int actualArgs = cadr(args);
    int first = car(actualArgs);
    int rest = cdr(actualArgs);

    if (type(first) == CONS)
        return defineFunction(car(first),cdr(first),rest,car(args));
    else if (type(first) == SYMBOL)
        return defineIdentifier(first,car(rest),car(args));
    else
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "can only define SYMBOLS, not type %s",
            SymbolTable[file(args)],line(args),
            type(first));
    }

/* (addSymbol name init env) */

static int
addSymbol(int args)
    {
    //debug("var: ",car(args));
    //debug("val: ",cadr(args));
    //debug("env: ",caddr(args));
    return defineVariable(caddr(args),car(args),cadr(args));
    }

/* (lambda # $params $) */

static int
lambda(int args)
    {
    int name = anonymousSymbol;
    int params = cadr(args);
    int body = caddr(args);

    return  makeClosure(car(args),name,params,body,ADD_BEGIN);
    }

static int
not(int args)
    {
    if (sameSymbol(car(args),trueSymbol)) return falseSymbol;
    if (sameSymbol(car(args),falseSymbol)) return trueSymbol;
    return throw(exceptionSymbol,
        "file %s,line %d: "
        "cannot logically negate type %s",
        SymbolTable[file(args)],line(args),
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
            result = ival(a) < ival(b);
        else if (aType == INTEGER && bType == REAL)
            result = ival(a) < rval(b);
        else if (aType == REAL && bType == INTEGER)
            result = rval(a) < ival(b);
        else if (aType == REAL && bType == REAL)
            result = rval(a) < rval(b);
        else
            return throw(exceptionSymbol,
                "file %s,line %d: "
                "wrong types for '<': %s and %s",
                SymbolTable[file(args)],line(args),
                aType,bType);

        if (!result) break;

        args = cdr(args);
        a = b;
        }

    result = scamBoolean(result);
    file(result) = file(args);
    line(result) = line(args);

    return result;
    }

/* (<= a b) */

static int
isLessThanOrEqualTo(int args)
    {
    int a,b,result;
    char *aType,*bType;
    int fi,li;

    args = car(args);

    if (args == 0) return scamBoolean(1);

    if (cdr(args) == 0) return scamBoolean(1);

    a = car(args);
    args = cdr(args);

    fi = file(a);
    li = line(a);

    result = 1;

    while (args != 0)
        {
        aType = type(a);
        b = car(args);
        bType = type(b);

        if (aType == INTEGER && bType == INTEGER)
            result = ival(a) <= ival(b);
        else if (aType == INTEGER && bType == REAL)
            result = ival(a) <= rval(b);
        else if (aType == REAL && bType == INTEGER)
            result = rval(a) <= ival(b);
        else if (aType == REAL && bType == REAL)
            result = rval(a) <= rval(b);
        else
            return throw(exceptionSymbol,
                "file %s,line %d: "
                "wrong types for '<=': %s and %s",
                SymbolTable[fi],li,
                aType,bType);

        if (!result) break;

        args = cdr(args);
        a = b;
        }

    result = scamBoolean(result);
    file(result) = fi;
    line(result) = li;

    return result;
    }

/* (= a b) */

static int
isNumericEqualTo(int args)
    {
    int a,b,result;
    char *aType,*bType;
    int fi,li;

    args = car(args);

    if (args == 0) return scamBoolean(1);

    if (cdr(args) == 0) return scamBoolean(1);

    a = car(args);
    args = cdr(args);

    fi = file(a);
    li = line(a);

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
            return throw(exceptionSymbol,
                "file %s,line %d: "
                "wrong types for '=': %s and %s",
                SymbolTable[fi],li,
                aType,bType);

        if (!result) break;

        args = cdr(args);
        a = b;
        }

    result = scamBoolean(result);
    file(result) = fi;
    line(result) = li;

    return result;
    }

/* (> a b) */

static int
isGreaterThan(int args)
    {
    int a,b,result;
    char *aType,*bType;
    int fi,li;

    args = car(args);

    if (args == 0) return scamBoolean(1);

    if (cdr(args) == 0) return scamBoolean(1);

    a = car(args);
    args = cdr(args);

    fi = file(a);
    li = line(a);

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
            return throw(exceptionSymbol,
                "file %s,line %d: "
                "wrong types for '>': %s and %s",
                SymbolTable[fi],li,
                aType,bType);

        if (!result) break;

        args = cdr(args);
        a = b;
        }

    result = scamBoolean(result);
    file(result) = fi;
    line(result) = li;

    return result;
    }


/* (>= a b) */

static int
isGreaterThanOrEqualTo(int args)
    {
    int a,b,result;
    char *aType,*bType;
    int fi,li;

    args = car(args);

    if (args == 0) return scamBoolean(1);

    if (cdr(args) == 0) return scamBoolean(1);

    a = car(args);
    args = cdr(args);

    fi = file(a);
    li = line(a);

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
            return throw(exceptionSymbol,
                "file %s,line %d: "
                "wrong types for '>=': %s and %s",
                SymbolTable[fi],li,
                aType,bType);

        if (!result) break;

        args = cdr(args);
        a = b;
        }

    result = scamBoolean(result);
    file(result) = fi;
    line(result) = li;

    return result;
    }

/* (eq? a b) */

static int
isEq(int args)
    {
    int a,b,result;
    char *aType,*bType;
    int fi,li;

    args = car(args);

    if (args == 0) return scamBoolean(1);

    if (cdr(args) == 0) return scamBoolean(1);

    a = car(args);
    args = cdr(args);

    fi = file(a);
    li = line(a);

    result = 1;

    while (args != 0)
        {
        aType = type(a);
        b = car(args);
        bType = type(b);
 
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
    file(result) = fi;
    line(result) = li;

    return result;
    }

/* (neq? a b) */

static int
isNotEq(int args)
    {
    int a,b,result;
    char *aType,*bType;
    int fi,li;

    args = car(args);

    if (args == 0) return scamBoolean(1);

    if (cdr(args) == 0) return scamBoolean(1);

    a = car(args);
    args = cdr(args);

    fi = file(a);
    li = line(a);

    result = 1;

    while (args != 0)
        {
        aType = type(a);
        b = car(args);
        bType = type(b);
 
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
    file(result) = fi;
    line(result) = li;

    return result;
    }

/* (+ @) */

static int
plus(int args)
    {
    int fi,li;
    int a,b,result;
    int itotal = 0;
    double rtotal = 0;
    char *oldType,*newType;

    args = car(args);

    if (args == 0) return newInteger(0);

    a = car(args);
    args = cdr(args);

    fi = file(a);
    li = line(a);

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
            return throw(exceptionSymbol,
                "file %s,line %d: "
                "wrong types for '+': %s and %s",
                SymbolTable[fi],li,
                oldType,newType);

        args = cdr(args);
        }

    if (oldType == INTEGER)
        result = newInteger(itotal);
    else if (oldType == REAL)
        result = newReal(rtotal);
    else
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "wrong types for '+': %s",
            SymbolTable[fi],li,
            oldType);

    file(result) = fi;
    line(result) = li;

    return result;
    }

/* (- @) */

static int
minus(int args)
    {
    int fi,li;
    int a,b,result;
    int itotal = 0;
    double rtotal = 0;
    char *oldType,*newType;

    args = car(args);

    if (args == 0) return newInteger(0);

    a = car(args);
    args = cdr(args);


    fi = file(a);
    li = line(a);

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
            return throw(exceptionSymbol,
                "file %s,line %d: "
                "wrong types for '-': %s and %s",
                SymbolTable[fi],li,
                oldType,newType);

        args = cdr(args);
        }

    if (oldType == INTEGER)
        result = newInteger(itotal);
    else if (oldType == REAL)
        result = newReal(rtotal);
    else
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "wrong types for '-': %s",
            SymbolTable[fi],li,
            oldType);

    file(result) = fi;
    line(result) = li;

    return result;
    }

/* (* @) */

static int
times(int args)
    {
    int fi,li;
    int a,b,result;
    int itotal = 0;
    double rtotal = 0;
    char *oldType,*newType;

    args = car(args);

    if (args == 0) return newInteger(1);

    a = car(args);
    args = cdr(args);

    fi = file(a);
    li = line(a);

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
            return throw(exceptionSymbol,
                "file %s,line %d: "
                "wrong types for '*': %s and %s",
                SymbolTable[fi],li,
                oldType,newType);

        args = cdr(args);
        }

    if (oldType == INTEGER)
        result = newInteger(itotal);
    else if (oldType == REAL)
        result = newReal(rtotal);
    else
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "wrong types for '*': %s",
            SymbolTable[fi],li,
            oldType);

    file(result) = fi;
    line(result) = li;

    return result;
    }

/* (/ @) */

static int
divides(int args)
    {
    int fi,li;
    int a,b,result;
    int itotal = 0;
    double rtotal = 0;
    char *oldType,*newType;

    args = car(args);

    if (args == 0) return newInteger(1);

    a = car(args);
    args = cdr(args);

    fi = file(a);
    li = line(a);

    oldType = type(a);
    itotal = ival(a);
    rtotal = rval(a);

    while (args != 0)
        {
        b = car(args);
        newType = type(b);

        if ((newType == INTEGER && ival(b) == 0)
        ||  (newType == REAL && rval(b) == 0.0))
            return throw(mathExceptionSymbol,
                "file %s,line %d: "
                "divide by zero",
                SymbolTable[fi],li
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
            return throw(exceptionSymbol,
                "file %s,line %d: "
                "wrong types for '/': %s and %s",
                SymbolTable[fi],li,
                oldType,newType);

        args = cdr(args);
        }

    if (oldType == INTEGER)
        result = newInteger(itotal);
    else if (oldType == REAL)
        result = newReal(rtotal);
    else
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "wrong types for '/': %s",
            SymbolTable[fi],li,
            oldType);

    file(result) = fi;
    line(result) = li;

    return result;
    }

/* (% @) */

static int
mod(int args)
    {
    int fi,li;
    int a,b,result;
    int itotal = 0;
    char *oldType,*newType;

    args = car(args);

    if (args == 0) return newInteger(0);

    a = car(args);
    args = cdr(args);

    fi = file(a);
    li = line(a);

    oldType = type(a);
    itotal = ival(a);

    while (args != 0)
        {
        b = car(args);
        newType = type(b);

        if (oldType == INTEGER && newType == INTEGER)
            itotal %= ival(b);
        else
            return throw(exceptionSymbol,
                "file %s,line %d: "
                "wrong types for '/': %s and %s",
                SymbolTable[fi],li,
                oldType,newType);

        args = cdr(args);
        }

    result = newInteger(itotal);

    file(result) = fi;
    line(result) = li;

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

    if (aType == INTEGER)
        result = newReal(exp(ival(a)));
    else if (aType == REAL)
        result = newReal(exp(rval(a)));
    else
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "wrong type for 'exp': %s",
            SymbolTable[file(car(args))],line(car(args)),
            aType);

    file(result) = file(car(args));
    line(result) = line(car(args));

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

    if (aType == INTEGER)
        result = newReal(log(ival(a)));
    else if (aType == REAL)
        result = newReal(log(rval(a)));
    else
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "wrong type for 'exp': %s",
            SymbolTable[file(car(args))],line(car(args)),
            aType);

    file(result) = file(car(args));
    line(result) = line(car(args));

    return result;
    }

/* (sin a) */

static int
ssin(int args)
    {
    int a,result;
    char *aType;

    a = car(args);
    aType = type(a);

    if (aType == INTEGER)
        result = newReal(sin(ival(a)));
    else if (aType == REAL)
        result = newReal(sin(rval(a)));
    else
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "wrong type for 'exp': %s",
            SymbolTable[file(car(args))],line(car(args)),
            aType);

    file(result) = file(car(args));
    line(result) = line(car(args));

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

    if (aType == INTEGER)
        result = newReal(cos(ival(a)));
    else if (aType == REAL)
        result = newReal(cos(rval(a)));
    else
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "wrong type for 'exp': %s",
            SymbolTable[file(car(args))],line(car(args)),
            aType);

    file(result) = file(car(args));
    line(result) = line(car(args));

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

    if (atype == INTEGER && btype == INTEGER)
        result = newReal(atan2(ival(a),ival(b)));
    else if (atype == INTEGER)
        result = newReal(atan2(ival(a),rval(b)));
    else if (btype == INTEGER)
        result = newReal(atan2(rval(a),ival(b)));
    else
        result = newReal(atan2(rval(a),rval(b)));

    file(result) = file(car(args));
    line(result) = line(car(args));

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

    if (atype == INTEGER && btype == INTEGER)
        result = newInteger((int) pow(ival(a),ival(b)));
    else if (atype == INTEGER)
        result = newReal(pow(ival(a),rval(b)));
    else if (btype == INTEGER)
        result = newReal(pow(rval(a),ival(b)));
    else
        result = newReal(pow(rval(a),rval(b)));

    file(result) = file(car(args));
    line(result) = line(car(args));

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
    //printf("in begin...\n");
    return evalList(cadr(args),car(args),ALLBUTLAST);
    }

/* (throw # $item) */

static int
rreturn(int args)
    {
    //printf("in return...\n");
    assureMemory("return",THUNK_CELLS + THROW_CELLS,&args,0);
    return makeThrow(returnSymbol,makeThunk(cadr(args),car(args)),0);
    }

/* (scope # $) */

static int
scope(int args)
    {
    int env;
    //printf("in scope...\n");
    assureMemory("scope",ENV_CELLS,&args,0);
    env = makeEnvironment(car(args),0,0,0);
    return evalList(cadr(args),env,ALLBUTLAST);
    }

/* (display item) */

static int
display(int args)
    {
    FILE *port = OpenPorts[CurrentOutputIndex];

    pp(port,car(args));

    return car(args);
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
        return throw(exceptionSymbol,
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
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "fmt given a non-number to be formatted: %s",
            SymbolTable[file(car(args))],line(car(args)),
            type(a));
        }

    return newString(buffer);
    }

static int
ppp(int args)
    {
    int a = car(args);
    FILE *port = OpenPorts[CurrentOutputIndex];

    if (type(a) == CONS && sameSymbol(car(a),objectSymbol))
        ppTable(port,a,0);
    else
        pp(port,a);
    return 0;
    }

/* (if # test $then $) */

static int
iif(int args)
    {
    //debug("if test",cadr(args));

    if (sameSymbol(cadr(args),falseSymbol))
        {
        //printf("if test is false\n");
        //debug("then",cadr(args));
        int otherwise = cadddr(args);
        if (otherwise != 0)
            {
            //debug("else",cadr(args));
            return makeThunk(car(otherwise),car(args));
            }
        else
            return 0;
        }
    else
        {
        //printf("if test is true\n");
        return makeThunk(caddr(args),car(args));
        }
    }

/* (cond # $) */

static int
cond(int args)
    {
    int cases = cadr(args);
    int env = car(args);

    while (cases != 0)
        {
        int result;
        int condition = caar(cases);

        if (sameSymbol(condition,elseSymbol))
            return  evalList(cdar(cases),env,ALLBUTLAST);

        push(env);
        push(cases);
        result = eval(condition,env);
        cases = pop();
        env = pop();

        rethrow(result,0);

        if (sameSymbol(result,trueSymbol))
            return  evalList(cdar(cases),env,ALLBUTLAST);

        cases = cdr(cases);
        }

    return falseSymbol;
    }

/* (while # $test $) */

static int
wwhile(int args)
    {
    int last = 0;
    int testResult;
    
    //printf("in while...\n");

    push(args);
    testResult = eval(cadr(args),car(args));
    args = pop();

    rethrow(testResult,0);

    while (sameSymbol(testResult,trueSymbol))
        {
        push(args);
        last = evalList(caddr(args),car(args),ALL);
        args = pop();

        rethrow(last,0);

        push(args);
        testResult = eval(cadr(args),car(args));
        args = pop();

        rethrow(testResult,0);

        //debug("test result",testResult);
        }

    return last;
    }

/* (set-car! spot value) */

static int
setCar(int args)
    {
    int a = car(args);
    int b = cadr(args);

    char *t = type(a);
    if (t != CONS && t != STRING && t != ARRAY)
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "attempt to set the car of type %s",
            SymbolTable[file(car(args))],line(car(args)),
            t);

    if (t == STRING)
        car(a) = ival(b);
    else
        car(a) = b;

    return b;
    }

/* (set-cdr! spot value) */

static int
setCdr(int args)
    {
    if (type(car(args)) != CONS)
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "attempt to set the cdr of type %s",
            SymbolTable[file(car(args))],line(car(args)),
            type(car(args)));

    cdar(args) = cadr(args);
    return cadr(args);
    }

/* (set! $id value # @) */

static int
set(int args)
    {
    int id = car(args);
    int result;
    
    //printf("in set!...");
    if (type(id) == CONS)
        {
        push(args);
        id = eval(id,caddr(args));
        args = pop();
        rethrow(id,0);
        }

    if (type(id) != SYMBOL)
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "set! identifier resolved to type %s, not SYMBOL",
            SymbolTable[file(id)],line(id),
            type(id));

    if (cadddr(args) == 0)
        result = setVariableValue(id,cadr(args),caddr(args));
    else
        result = setVariableValue(id,cadr(args),car(cadddr(args)));

    //debug("set! returning",result);
    return result;
    }

/* (assign $id value # @) */

static int
assign(int args)
    {
    int id = car(args);
    int obj;
    int result;
    
    //printf("in assign...");
    if (type(id) == SYMBOL)
        return set(args);

    //printf("type of id is %s\n",type(id));
    if (type(id) != CONS
    || !(sameSymbol(car(id),dotSymbol) || sameSymbol(car(id),DOTSymbol)))
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "cannot assign to type %s",
            SymbolTable[file(car(args))],line(car(args)),
            type(id));

    obj = cadr(id);
    id = caddr(id);

    push(args);
    push(id);
    obj = eval(obj,caddr(args));
    id = pop();
    args = pop();

    rethrow(obj,0);

    if (type(id) == CONS)
        {
        //debug("calling",id);
        push(args);
        push(obj);
        id = eval(id,caddr(args));
        obj = pop();
        args = pop();
        //debug("id now is",id);
        }

    result = setVariableValue(id,cadr(args),obj);

    return result;
    }


/* (get $id # @) */

static int 
get(int args)
    {
    int id = car(args);

    //printf("in get...");
    if (type(id) == CONS)
        {
        push(args);
        id = eval(id,cadr(args));
        args = pop();
        rethrow(id,0);
        }

    if (type(id) != SYMBOL)
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "get variable argument resolved to type %s, not SYMBOL",
            SymbolTable[file(args)],line(args),
            type(id));

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

    push(args);
    result = eval(cadr(args),car(args));
    args = pop();

    pp(stdout,cadr(args));
    fprintf(stdout," is ");
    pp(stdout,result);
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

    if (type(fileName) != STRING)
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "include argument was type %s, not STRING",
            SymbolTable[file(args)],line(args),
            type(fileName));

    cellString(buffer,sizeof(buffer),fileName);

    //check to see if file has already been included
    //if so, return false

    sprintf(buffer2,"__included_%s",buffer);

    s = newSymbol(buffer2);

    if (isLocal(s,env)) return falseSymbol;

    //printf("file %s not already included\n",buffer);

    assureMemory("include",DEFINE_CELLS,&env,&s,0);
    defineVariable(env,s,trueSymbol);
    //printf("%s defined in env %d\n",buffer2,env);

    push(env);

    p = newParser(buffer);
    if (p == 0)
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "file %s could not be opened for reading",
            SymbolTable[file(args)],line(args),
            buffer);

    ptree = parse(p);
    fclose(p->input);
    free(p);

    env = pop();

    rethrow(ptree,0);

    //debug("include",ptree);

    return makeThunk(ptree,env);
    }

/* (eval expr context) */

static int
eeval(int args)
    {
    return eval(car(args),cadr(args));
    }

/* (evalList items context) */

static int
eevalList(int args)
    {
    int result;
    int items = car(args);
    while (items != 0)
        {
        result = eval(car(items),cadr(args));
        items = cdr(items);
        }
    return result;
    }

/* (apply f args) */

static int
apply(int args)
    {
    assureMemory("apply:",1,&args,0);

    return evalCall(cons(car(args),cadr(args)),0,NO_EVALUATION);
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
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "attempt to take car of type %s",
            SymbolTable[file(supply)],line(supply),
            t);

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

/* (cdr item) */

static int
ccdr(int args)
    {
    return cdr(car(args));
    }

/* (cons a b) */

static int
ccons(int args)
    {
    assureMemory("cons",1,&args,0);
    return ucons(car(args),cadr(args));
    }

/* (gensym id) */

static int
gensym(int args)
    {
    int result;
    char buffer[512];
    static int count = 0;

    if (car(args) == 0)
        snprintf(buffer,sizeof(buffer),"%dsym",count++);
    else
        snprintf(buffer,sizeof(buffer),
            "%d%s",count++,SymbolTable[ival(caar(args))]);

    result = newSymbol(buffer);
    //debug("gensym",result);
    return result;
    }

/* (gensym? id) */

static int
isGensym(int args)
    {
    int id = car(args);

    if (type(id) != SYMBOL) return falseSymbol;

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
addOpenPort(FILE *fp,int portType,int fi,int li)
    {
    int i;
    int maxPorts = sizeof(OpenPorts) / sizeof(FILE *);

    for (i = 2; i < maxPorts; ++i)
        {
        //printf("port[%d] is %p\n",i,OpenPorts[i]);
        if (OpenPorts[i] == 0)
            break;
        }

    //printf("first available port is %d\n",i);

    if (i == maxPorts)
        {
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "too many ports open at once",
            SymbolTable[fi],li
            );
        }

    OpenPorts[i] = fp;

    assureMemory("addOpenPort",3,0);

    return ucons(portType,ucons(newInteger(i),0));
    }

static int
setPort(int args)
    {
    int old;
    int target;

    assureMemory("setPort",3,&args,0);

    target = car(args);

    if (type(target) == SYMBOL)
        {
        if (ival(target) == stdinIndex)
            {
            old = CurrentInputIndex;
            CurrentInputIndex = 0;
            return ucons(inputPortSymbol,ucons(newInteger(old),0));
            }
        else if (ival(target) == stdoutIndex)
            {
            old = CurrentOutputIndex;
            CurrentOutputIndex = 1;
            return ucons(outputPortSymbol,ucons(newInteger(old),0));
            }
        else 
            return throw(exceptionSymbol,
                "file %s,line %d: "
                "%s is not a valid argument to setPort",
                SymbolTable[file(args)],line(args),
                SymbolTable[ival(target)]);
        }
    else if (type(target) == CONS && sameSymbol(car(target),inputPortSymbol))
        {
        old = CurrentInputIndex;
        CurrentInputIndex = ival(cadr(target));
        return uconsfl(inputPortSymbol,ucons(newInteger(old),0),
            file(args),line(args));
        }
    else if (type(target) == CONS && sameSymbol(car(target),outputPortSymbol))
        {
        old = CurrentOutputIndex;
        CurrentOutputIndex = ival(cadr(target));
        return uconsfl(outputPortSymbol,ucons(newInteger(old),0),
            file(args),line(args));
        }

    return throw(exceptionSymbol,
        "file %s,line %d: "
        "setPort given a non-port as argument: %s",
        SymbolTable[file(args)],line(args),
        type(target));
    }

static int
getInputPort(int args)
    {
    assureMemory("getInputPort",3,0);
    return ucons(inputPortSymbol,ucons(newInteger(CurrentInputIndex),0));
    }

static int
getOutputPort(int args)
    {
    assureMemory("getOutputPort",3,0);
    return ucons(outputPortSymbol,ucons(newInteger(CurrentOutputIndex),0));
    }

static int
checkValidPort(int index,int fi,int li)
    {
    if (index == 0)
        {
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "attempt to close stdin",
            SymbolTable[fi],li
            );
        }
    if (index == 1)
        {
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "attempt to close stdout",
            SymbolTable[fi],li
            );
        }
    if (index >= MaxPorts || index < 0)
        {
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "attempt to close a non-existent port: %d",
            SymbolTable[fi],li,
            index);
        }
    if (OpenPorts[index] == 0)
        {
        return throw(exceptionSymbol,
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
        if (sameSymbol(car(target),inputPortSymbol))
            newPort = 0;
        else if (sameSymbol(car(target),outputPortSymbol))
            newPort = 1;
        else
            return throw(exceptionSymbol,
                "file %s,line %d: "
                "close passed a malformed port",
                SymbolTable[file(args)],line(args)
                );

        if (cdr(target) == 0 || type(cadr(target)) != INTEGER)
            return throw(exceptionSymbol,
                "file %s,line %d: "
                "close passed a malformed port",
                SymbolTable[file(args)],line(args)
                );

        index = ival(cadr(target));
        rethrow(checkValidPort(index,file(args),line(args)),0);
        fclose(OpenPorts[index]);
        OpenPorts[index] = 0;
        if (CurrentInputIndex == index) CurrentInputIndex = newPort;
        }
    else
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "close passed a malformed port",
            SymbolTable[file(args)],line(args)
            );

    return trueSymbol;
    }

static int
checkPortForReading(FILE *fp,char *item,int args)
    {
    if (fp == 0)
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "attempt to read %s from a closed port",
            SymbolTable[file(args)],line(args),
            item
            );

    if (feof(fp))
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "attempt to read %s at end of input",
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

    rethrow(checkPortForReading(fp,"a character",args),0);

    ch = fgetc(fp);

    if (feof(fp)) 
        return eofSymbol;

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

    rethrow(checkPortForReading(fp,"a character",args),0);

    i = 0;
    if (fscanf(fp," %c",&i) == EOF) return eofSymbol;
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

    rethrow(checkPortForReading(fp,"an integer",args),0);

    i = 0;
    if (fscanf(fp," %d",&i) == EOF) return eofSymbol;
    return newInteger(i);
    }

static int
readReal(int args)
    {
    double r;
    FILE *fp;

    fp = OpenPorts[CurrentInputIndex];

    rethrow(checkPortForReading(fp,"a real",args),0);

    r = 0;
    if (fscanf(fp," %lf",&r) == EOF) return eofSymbol;
    return newReal(r);
    }

static int
readString(int args)
    {
    int ch;
    int index;
    char buffer[4096];
    int result;
    int backslashed;
    FILE *fp;

    fp = OpenPorts[CurrentInputIndex];

    rethrow(checkPortForReading(fp,"a string",args),0);

    skipWhiteSpace(fp);

    ch = fgetc(fp);
    if (ch != '\"')
        {
        ungetc(ch,fp);
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "found <%c> instead of double quote at the start of a string",
            SymbolTable[file(args)],line(args),
            ch);
        }

    index = 0;
    backslashed = 0;
    while ((ch = fgetc(fp)) && ch != EOF)
        {
        if (ch == '\"')
            break;

        if (ch == '\\')
            {
            ch = fgetc(fp);
            if (ch == EOF)
                return throw(exceptionSymbol,
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
            return throw(exceptionSymbol,
                "file %s,line %d: "
                "attempt to read a very long string failed",
                SymbolTable[file(args)],line(args)
                );
        }

    buffer[index] = '\0';

    if (ch != '\"')
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "attempt to read an unterminated string",
            SymbolTable[file(args)],line(args)
            );

    //printf("string is <%s>\n", buffer);

    result = newString(buffer);

    return result;
    }

static int
readToken(int args)
    {
    int ch;
    int index;
    char buffer[4096];
    int result;
    FILE *fp;

    fp = OpenPorts[CurrentInputIndex];

    rethrow(checkPortForReading(fp,"a token",args),0);

    skipWhiteSpace(fp);

    index = 0;
    while ((ch = fgetc(fp)) && ch != EOF && !isspace(ch))
        {
        buffer[index++] = ch;
        if (index == sizeof(buffer) - 1)
            return throw(exceptionSymbol,
                "file %s,line %d: "
                "attempt to read a very long token failed",
                SymbolTable[file(args)],line(args)
                );
        }

    buffer[index] = '\0';

    ungetc(ch,fp);

    //printf("token is <%s>\n", buffer);

    result = newString(buffer);

    return result;
    }

static int
readWhile(int args)
    {
    int ch;
    int a = car(args);
    int index;
    char target[256];
    char buffer[4096];
    int result;
    FILE *fp;

    if (type(a) != STRING)
        {
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "readWhile argument should be STRING, not type %s",
            SymbolTable[file(args)],line(args),
            type(a));
        }

    fp = OpenPorts[CurrentInputIndex];

    rethrow(checkPortForReading(fp,"characters",args),0);

    cellStringTr(target,sizeof(target),a);

    index = 0;
    while ((ch = fgetc(fp)) && ch != EOF && strchr(target,ch) != 0)
        {
        buffer[index++] = ch;
        if (index == sizeof(buffer) - 1)
            return throw(exceptionSymbol,
                "file %s,line %d: "
                "attempt to read a very long token failed",
                SymbolTable[file(args)],line(args)
                );
        }

    buffer[index] = '\0';

    ungetc(ch,fp);

    //printf("token is <%s>\n", buffer);

    result = newString(buffer);

    return result;
    }

static int
readUntil(int args)
    {
    int ch;
    int a = car(args);
    int index;
    char target[256];
    char buffer[4096];
    int result;
    FILE *fp;

    if (type(a) != STRING)
        {
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "readWhile argument should be STRING, not type %s",
            SymbolTable[file(args)],line(args),
            type(a));
        }

    fp = OpenPorts[CurrentInputIndex];

    rethrow(checkPortForReading(fp,"characters",args),0);

    cellStringTr(target,sizeof(target),a);

    index = 0;
    while ((ch = fgetc(fp)) && ch != EOF && strchr(target,ch) == 0)
        {
        buffer[index++] = ch;
        if (index == sizeof(buffer) - 1)
            return throw(exceptionSymbol,
                "file %s,line %d: "
                "attempt to read a very long token failed",
                SymbolTable[file(args)],line(args)
                );
        }

    buffer[index] = '\0';

    ungetc(ch,fp);

    //printf("token is <%s>\n", buffer);

    result = newString(buffer);

    return result;
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

    return throw(exceptionSymbol,
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
    int result;
    FILE *fp;

    fp = OpenPorts[CurrentInputIndex];

    rethrow(checkPortForReading(fp,"a line",args),0);

    index = 0;
    while ((ch = fgetc(fp)) && ch != EOF && ch != '\n')
        {
        buffer[index++] = ch;
        if (index == sizeof(buffer) - 1)
            return throw(exceptionSymbol,
                "file %s,line %d: "
                "attempt to read a very long line failed",
                SymbolTable[file(args)],line(args)
                );
        }

    buffer[index] = '\0';

    //printf("token is <%s>\n", buffer);

    result = newString(buffer);

    return result;
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
            FILE *fp = fopen(cellString(buffer,sizeof(buffer),target),"r");
            if (fp == 0)
                return throw(exceptionSymbol,
                    "file %s,line %d: "
                    "file %s cannot be opened for reading",
                    SymbolTable[file(args)],line(args),
                    buffer);
            result = addOpenPort(fp,inputPortSymbol,file(args),line(args));
            }
        else if (ival(mode) == writeIndex)
            {
            char buffer[256];
            FILE *fp = fopen(cellString(buffer,sizeof(buffer),target),"w");
            //printf("buffer is %s\n",buffer);
            if (fp == 0)
                return throw(exceptionSymbol,
                    "file %s,line %d: "
                    "file %s cannot be opened for writing",
                    SymbolTable[file(args)],line(args),
                    buffer);
            //printf("file opened successfully\n");
            result = addOpenPort(fp,outputPortSymbol,file(args),line(args));
            }
        else if (ival(mode) == appendIndex)
            {
            char buffer[256];
            FILE *fp = fopen(cellString(buffer,sizeof(buffer),target),"a");
            if (fp == 0)
                return throw(exceptionSymbol,
                    "file %s,line %d: "
                    "file %s cannot be opened for appending",
                    SymbolTable[file(args)],line(args),
                    buffer);
            result = addOpenPort(fp,outputPortSymbol,file(args),line(args));
            }
        else 
            {
            return throw(exceptionSymbol,
                "file %s,line %d: "
                "%s is an unknown mode "
                "(should be 'read, 'write, or 'append)",
                SymbolTable[file(args)],line(args),
                SymbolTable[ival(mode)]);
            }
        }
    else
        {
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "%s is an unknown mode "
            "(should be 'read, 'write, or 'append)",
            SymbolTable[file(args)],line(args),
            type(mode));
        }

    //debug("new port",result);
    return result;
    }

static int
ttime(int args)
    {
    return newInteger((int) time(0));
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

    assureMemory("bindings",4 * length(vars),0);

    items = 0;
    while (vars != 0)
        {
        int pack = ucons(ucons(car(vars),ucons(car(vals),0)),0);
        if (items == 0)
            {
            items = pack;
            spot = items;
            }
        else
            {
            cdr(spot) = pack;
            spot = cdr(spot);
            }
        vars = cdr(vars);
        vals = cdr(vals);
        }

    return items;
    }

/* (array @) */

static int
array(int args)
    {
    int i,size,amount,spot,start = 0,attach = 0;
    args = car(args);
    size = length(args);
    amount = size;
    assureMemory("array",size,&args,0);
    for (i = 0; i < size; ++i)
        {
        spot = cons(car(args),0);
        type(spot) = ARRAY;
        count(spot) = amount--;
        if (i == 0)
            start = spot;
        else
            cdr(attach) = spot;
        attach = spot;
        args = cdr(args);
        }
    //printf("size of array is %d\n",count(start));
    return start;
    }

/* (allocate size) */

static int
allocate(int args)
    {
    int i,spot,start = 0,attach = 0;
    int size = ival(car(args));
    int amount = size;
    assureMemory("allocate",size,0);
    for (i = 0; i < size; ++i)
        {
        spot = cons(0,0);
        type(spot) = ARRAY;
        count(spot) = amount--;
        if (i == 0)
            start = spot;
        else
            cdr(attach) = spot;
        attach = spot;
        }
    return start;
    }

/* (length item) */

static int
llength(int args)
    {
    int item = car(args);

    if (type(item) == ARRAY)
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
    else
        return throw(exceptionSymbol,
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
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "negative indices (%d) are not allowed",
            SymbolTable[file(args)],line(args),
            index);

    if (kind == ARRAY)
        limit = count(supply);
    else if (kind == STRING || kind == CONS)
        limit = length(supply);
    else
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "cannot index into type %s",
            SymbolTable[file(args)],line(args),
            kind);

    if (index >= limit)
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "index (%d) is too large",
            SymbolTable[file(args)],line(args),
            index);

    if (type(supply) == ARRAY)
        {
        //printf("getting an element of an array\n");
        return car(supply + index);
        }
    else
        {
        //printf("getting an element of a list or string\n");
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
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "negative indices (%d) are not allowed",
            SymbolTable[file(args)],line(args),
            index);

    if (kind == ARRAY)
        limit = count(supply);
    else if (kind == STRING || kind == CONS)
        limit = length(supply);
    else
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "cannot index into type %s",
            SymbolTable[file(args)],line(args),
            kind);

    if (index >= limit)
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "index (%d) is too large",
            SymbolTable[file(args)],line(args),
            index);

    if (type(supply) == ARRAY)
        {
        //printf("getting an element of an array\n");
        car(supply + index) = value;
        return car(supply + index);
        }
    else
        {
        //printf("getting an element of a list or string\n");
        while (index > 0)
            {
            supply = cdr(supply);
            --index;
            }

        if (type(supply) == CONS)
            car(supply) = value;
        else
            ival(supply) = ival(value);

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

    //printf("in catch...\n");
    result = eval(cadr(args),car(args));
    //debug("caught",result);

    if (isThrow(result))
        {
        object_label(result) = errorSymbol;
        }

    return result;
    }

/* (throw code @) */

static int
tthrow(int args)
    {
    int item;

    assureMemory("throw",THROW_CELLS,&args,0);

    item = car(args);

    if (cadr(args) == 0 && isError(item))
        return makeThrow(error_code(item),error_value(item),error_trace(item));
    else if (cadr(args) != 0)
        return makeThrow(item,car(cadr(args)),0);
    else
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "wrong number of arguments to throw",
            SymbolTable[file(args)],line(args)
            );
    }

/* string manipulations */

/* (string+ a b) */

static int
string_plus(int args)
    {
    int a = car(args);
    int b = cadr(args);
    int i,amount,sizeA,sizeB,size,attach,spot,start;

    if (a != 0 && type(a) != STRING)
        {
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "prefix: first argument should be STRING, not %s",
            SymbolTable[file(args)],line(args),
            type(a));
        }

    if (b != 0 && type(b) != STRING)
        {
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "prefix: first argument should be STRING, not %s",
            SymbolTable[file(args)],line(args),
            type(b));
        }

    if (a == 0 && b == 0) return 0;

    sizeA = length(a);
    sizeB = length(b);
    size = sizeA + sizeB;

    assureMemory("string_plus",size,0);

    i = 0;
    amount = size;
    for (i = 0; i < size; ++i)
        {
        spot = cons(0,0);
        type(spot) = STRING;
        if (i < sizeA)
            {
            ival(spot) = ival(a);
            a = cdr(a);
            }
        else
            {
            ival(spot) = ival(b);
            b = cdr(b);
            }
        count(spot) = amount--;
        if (i == 0)
            start = spot;
        else
            cdr(attach) = spot;
        attach = spot;
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
        if (b == 0) return falseSymbol;
        if (ival(car(a)) != ival(car(b))) return falseSymbol;
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
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "prefix: first argument should be STRING, not %s",
            SymbolTable[file(args)],line(args),
            type(a));
        }

    if (type(b) != INTEGER)
        {
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "prefix: second argument should be INTEGER, not %s",
            SymbolTable[file(args)],line(args),
            type(b));
        }


    buffer = (char *) New(ival(b) + 1);

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
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "suffix: first argument should be STRING, not %s",
            SymbolTable[file(args)],line(args),
            type(a));
        }

    if (type(b) != INTEGER)
        {
        return throw(exceptionSymbol,
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
        return throw(exceptionSymbol,
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
        return throw(exceptionSymbol,
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
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "substring: first argument is type %s (should be %s)",
            SymbolTable[file(args)],line(args),
            type(needle),STRING);
        }

    if (type(haystack) != STRING)
        {
        return throw(exceptionSymbol,
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
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "trim: argument is type %s (should be %s)",
            SymbolTable[file(args)],line(args),
            type(a),STRING);
        }

    assureMemory("trim",count(a) + 1,&a,0);

    start = MemorySpot;

    while (isspace(ival(a)))
        a = cdr(a);

    assert(ival(a + count(a)) == 0);

    length = count(a) - 1;
    while (isspace(ival(a + length)))
        --length;

    while (length >= 0)
        {
        ucons(ival(a),MemorySpot+1);
        a = cdr(a);
        --length;
        }

    ucons(0,0);
    assert(MemorySpot < MemorySize - 1);

    return start;
    }
        
static int
ascii(int args)
    {
    int a = car(args);

    if (type(a) != STRING)
        {
        return throw(exceptionSymbol,
            "file %s,line %d: "
            "ascii: argument is type %s (should be %s)",
            SymbolTable[file(args)],line(args),
            type(a),STRING);
        }

    return newInteger(ival(a));
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

    return throw(exceptionSymbol,
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

    return throw(exceptionSymbol,
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

    return throw(exceptionSymbol,
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

/* (stack-depth) */

static int
stackDepth(int args)
    {
    return newInteger(StackPtr);
    }

void
loadBuiltIns(int env)
    {
    int b;
    int count = 0;

    BuiltIns[count] = ggc;
    b = makeBuiltIn(env,
        newSymbol("gc"),
        0,
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = string_plus;
    b = makeBuiltIn(env,
        newSymbol("string+"),
        ucons(newSymbol("a"),
            ucons(newSymbol("b"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = string_equal;
    b = makeBuiltIn(env,
        newSymbol("string-equal?"),
        ucons(newSymbol("a"),
            ucons(newSymbol("b"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = prefix;
    b = makeBuiltIn(env,
        newSymbol("prefix"),
        ucons(newSymbol("str"),
            ucons(newSymbol("size"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = suffix;
    b = makeBuiltIn(env,
        newSymbol("suffix"),
        ucons(newSymbol("str"),
            ucons(newSymbol("size"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = stringUntil;
    b = makeBuiltIn(env,
        newSymbol("stringUntil"),
        ucons(newSymbol("expr"),
            ucons(newSymbol("chars"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = stringWhile;
    b = makeBuiltIn(env,
        newSymbol("stringWhile"),
        ucons(newSymbol("str"),
            ucons(newSymbol("chars"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = substring;
    b = makeBuiltIn(env,
        newSymbol("substring"),
        ucons(newSymbol("needle"),
            ucons(newSymbol("haystack"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = trim;
    b = makeBuiltIn(env,
        newSymbol("trim"),
        ucons(newSymbol("str"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ascii;
    b = makeBuiltIn(env,
        newSymbol("ascii"),
        ucons(newSymbol("str"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = iint;
    b = makeBuiltIn(env,
        newSymbol("int"),
        ucons(newSymbol("item"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = rreal;
    b = makeBuiltIn(env,
        newSymbol("real"),
        ucons(newSymbol("item"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = sstring;
    b = makeBuiltIn(env,
        newSymbol("string"),
        ucons(newSymbol("item"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = catch;
    b = makeBuiltIn(env,
        newSymbol("catch"),
        ucons(sharpSymbol,
            ucons(newSymbol("$expr"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = tthrow;
    b = makeBuiltIn(env,
        newSymbol("throw"),
        ucons(newSymbol("code"),
            ucons(atSymbol,0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = symbol;
    b = makeBuiltIn(env,
        newSymbol("symbol"),
        ucons(newSymbol("str"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = getElement;
    b = makeBuiltIn(env,
        newSymbol("getElement"),
        ucons(newSymbol("item"),
            ucons(newSymbol("index"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = setElement;
    b = makeBuiltIn(env,
        newSymbol("setElement"),
        ucons(newSymbol("item"),
            ucons(newSymbol("index"),
                ucons(newSymbol("value"),0))),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = llength;
    b = makeBuiltIn(env,
        newSymbol("length"),
        ucons(newSymbol("item"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = allocate;
    b = makeBuiltIn(env,
        newSymbol("allocate"),
        ucons(newSymbol("size"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = array;
    b = makeBuiltIn(env,
        newSymbol("array"),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = bindings;
    b = makeBuiltIn(env,
        newSymbol("bindings"),
        ucons(newSymbol("object"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ssystem;
    b = makeBuiltIn(env,
        newSymbol("system"),
        ucons(newSymbol("str"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = eexec;
    b = makeBuiltIn(env,
        newSymbol("exec"),
        ucons(newSymbol("str"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ttime;
    b = makeBuiltIn(env,
        newSymbol("time"),
        0,
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readChar;
    b = makeBuiltIn(env,
        newSymbol("readChar"),
        ucons(sharpSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readRawChar;
    b = makeBuiltIn(env,
        newSymbol("readRawChar"),
        ucons(sharpSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readInt;
    b = makeBuiltIn(env,
        newSymbol("readInt"),
        ucons(sharpSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readReal;
    b = makeBuiltIn(env,
        newSymbol("readReal"),
        ucons(sharpSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readString;
    b = makeBuiltIn(env,
        newSymbol("readString"),
        ucons(sharpSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readToken;
    b = makeBuiltIn(env,
        newSymbol("readToken"),
        ucons(sharpSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readLine;
    b = makeBuiltIn(env,
        newSymbol("readLine"),
        ucons(sharpSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readWhile;
    b = makeBuiltIn(env,
        newSymbol("readWhile"),
        ucons(newSymbol("string"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readUntil;
    b = makeBuiltIn(env,
        newSymbol("readUntil"),
        ucons(newSymbol("string"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = pushBack;
    b = makeBuiltIn(env,
        newSymbol("pushBack"),
        ucons(newSymbol("string"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = oopen;
    b = makeBuiltIn(env,
        newSymbol("open"),
        ucons(newSymbol("name"),
            ucons(newSymbol("mode"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = setPort;
    b = makeBuiltIn(env,
        newSymbol("setPort"),
        ucons(newSymbol("port"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = getOutputPort;
    b = makeBuiltIn(env,
        newSymbol("getOutputPort"),
        ucons(sharpSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = getInputPort;
    b = makeBuiltIn(env,
        newSymbol("getInputPort"),
        ucons(sharpSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = cclose;
    b = makeBuiltIn(env,
        newSymbol("close"),
        ucons(newSymbol("port"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isEof;
    b = makeBuiltIn(env,
        newSymbol("eof?"),
        ucons(sharpSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = gensym;
    b = makeBuiltIn(env,
        newSymbol("gensym"),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isGensym;
    b = makeBuiltIn(env,
        newSymbol("gensym?"),
        ucons(newSymbol("id"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ccons;
    b = makeBuiltIn(env,
        newSymbol("cons"),
        ucons(newSymbol("a"),
            ucons(newSymbol("b"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

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

    BuiltIns[count] = ttype;
    b = makeBuiltIn(env,
        newSymbol("type"),
        ucons(newSymbol("item"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isNull;
    b = makeBuiltIn(env,
        newSymbol("null?"),
        ucons(newSymbol("item"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isPair;
    b = makeBuiltIn(env,
        newSymbol("pair?"),
        ucons(newSymbol("item"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isList;
    b = makeBuiltIn(env,
        newSymbol("list?"),
        ucons(newSymbol("item"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = list;
    b = makeBuiltIn(env,
        newSymbol("list"),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = iinclude;
    b = makeBuiltIn(env,
        newSymbol("include"),
        ucons(sharpSymbol,
            ucons(newSymbol("$fileName"),0)),
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

    BuiltIns[count] = eevalList;
    b = makeBuiltIn(env,
        newSymbol("evalList"),
        ucons(newSymbol("expr"),
            ucons(newSymbol("context"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = apply;
    b = makeBuiltIn(env,
        newSymbol("apply"),
        ucons(newSymbol("f"),
            ucons(newSymbol("args"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;


    BuiltIns[count] = inspect;
    b = makeBuiltIn(env,
        newSymbol("inspect"),
        ucons(sharpSymbol,
            ucons(newSymbol("$expr"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = scope;
    b = makeBuiltIn(env,
        newSymbol("scope"),
        ucons(sharpSymbol,
            ucons(dollarSymbol,0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = begin;
    b = makeBuiltIn(env,
        beginSymbol,
        ucons(sharpSymbol,
            ucons(dollarSymbol,0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = rreturn;
    b = makeBuiltIn(env,
        returnSymbol,
        ucons(sharpSymbol,
            ucons(newSymbol("$item"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = define;
    b = makeBuiltIn(env,
        newSymbol("define"),
        ucons(sharpSymbol,
            ucons(dollarSymbol,0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = addSymbol;
    b = makeBuiltIn(env,
        newSymbol("addSymbol"),
        ucons(newSymbol("name"),
            ucons(newSymbol("init"),
                ucons(newSymbol("env"),0))),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = display;
    b = makeBuiltIn(env,
        newSymbol("display"),
        ucons(newSymbol("item"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = fmt;
    b = makeBuiltIn(env,
        newSymbol("fmt"),
        ucons(newSymbol("format"),
            ucons(valueSymbol,0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ppp;
    b = makeBuiltIn(env,
        newSymbol("pp"),
        ucons(newSymbol("a"),0),
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

    BuiltIns[count] = minus;
    b = makeBuiltIn(env,
        newSymbol("-"),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = times;
    b = makeBuiltIn(env,
        newSymbol("*"),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = divides;
    b = makeBuiltIn(env,
        newSymbol("/"),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = mod;
    b = makeBuiltIn(env,
        newSymbol("%"),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = eexp;
    b = makeBuiltIn(env,
        newSymbol("exp"),
        ucons(newSymbol("n"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = llog;
    b = makeBuiltIn(env,
        newSymbol("log"),
        ucons(newSymbol("n"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = ssin;
    b = makeBuiltIn(env,
        newSymbol("sin"),
        ucons(newSymbol("n"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = coss;
    b = makeBuiltIn(env,
        newSymbol("cos"),
        ucons(newSymbol("n"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = aatan;
    b = makeBuiltIn(env,
        newSymbol("atan"),
        ucons(newSymbol("y"),
            ucons(newSymbol("x"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = expt;
    b = makeBuiltIn(env,
        newSymbol("expt"),
        ucons(newSymbol("base"),
            ucons(newSymbol("exponent"),0)),
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
        ucons(sharpSymbol,
            ucons(newSymbol("$params"),
                ucons(dollarSymbol,0))),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = cond;
    b = makeBuiltIn(env,
        newSymbol("cond"),
        ucons(sharpSymbol,
            ucons(dollarSymbol,0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = iif;
    b = makeBuiltIn(env,
        newSymbol("if"),
        ucons(sharpSymbol,
            ucons(newSymbol("test"),
                ucons(newSymbol("$then"),
                    ucons(dollarSymbol,0)))),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = wwhile;
    b = makeBuiltIn(env,
        newSymbol("while"),
        ucons(sharpSymbol,
            ucons(newSymbol("$test"),
                ucons(dollarSymbol,0))),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = address;
    b = makeBuiltIn(env,
        newSymbol("address"),
        ucons(newSymbol("item"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = not;
    b = makeBuiltIn(env,
        newSymbol("not"),
        ucons(valueSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isLessThan;
    b = makeBuiltIn(env,
        newSymbol("<"),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isLessThanOrEqualTo;
    b = makeBuiltIn(env,
        newSymbol("<="),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isGreaterThan;
    b = makeBuiltIn(env,
        newSymbol(">"),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isGreaterThanOrEqualTo;
    b = makeBuiltIn(env,
        newSymbol(">="),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isEq;
    b = makeBuiltIn(env,
        newSymbol("eq?"),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isEq;
    b = makeBuiltIn(env,
        newSymbol("=="),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isNotEq;
    b = makeBuiltIn(env,
        newSymbol("neq?"),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isNumericEqualTo;
    b = makeBuiltIn(env,
        newSymbol("="),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isNotEq;
    b = makeBuiltIn(env,
        newSymbol("!="),
        ucons(atSymbol,0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = get;
    b = makeBuiltIn(env,
        newSymbol("get"),
        ucons(newSymbol("$id"),
            ucons(sharpSymbol,
                ucons(atSymbol,0))),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = set;
    b = makeBuiltIn(env,
        newSymbol("set!"),
        ucons(newSymbol("$id"),
            ucons(valueSymbol,
                ucons(sharpSymbol,
                    ucons(atSymbol,0)))),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = assign;
    b = makeBuiltIn(env,
        newSymbol("assign"),
        ucons(newSymbol("$id"),
            ucons(valueSymbol,
                ucons(sharpSymbol,
                    ucons(atSymbol,0)))),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;


    BuiltIns[count] = setCar;
    b = makeBuiltIn(env,
        newSymbol("set-car!"),
        ucons(newSymbol("spot"),
            ucons(valueSymbol,0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = setCdr;
    b = makeBuiltIn(env,
        newSymbol("set-cdr!"),
        ucons(newSymbol("spot"),
            ucons(valueSymbol,0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = stackDepth;
    b = makeBuiltIn(env,
        newSymbol("stack-depth"),
        0,
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    assert(count <= sizeof(BuiltIns) / sizeof(PRIM));

    OpenPorts[0] = stdin;
    OpenPorts[1] = stdout;
    CurrentInputIndex = 0;
    CurrentOutputIndex = 1;
    }

void
installArgsEnv(int argc, char **argv, char **envv, int env)
    {
    int index;
    int start;

    //printf("installing command line arguments and execution environment\n");

    if (argc > 0)
        {
        start = MemorySpot;
        MemorySpot += argc;
        index = 0;

        while (index < argc)
            {
            type(start+index) = ARRAY;
            count(start+index) = argc - index;
            car(start+index) = newString(argv[index]);
            if (index == argc - 1)
                cdr(start+index) = 0;
            else
                cdr(start+index) = start+index + 1;
            ++index;
            }

        defineVariable(env,newSymbol(ArgumentsName),start);
        }
    else
        {
        defineVariable(env,newSymbol(ArgumentsName),0);
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
        start = MemorySpot;
        MemorySpot += argc;
        index = 0;

        while (index < argc)
            {
            type(start+index) = ARRAY;
            count(start+index) = argc - index;
            car(start+index) = newString(envv[index]);
            if (index == argc - 1)
                cdr(start+index) = 0;
            else
                cdr(start+index) = start+index + 1;
            ++index;
            }

        defineVariable(env,newSymbol(EnvironmentName),start);
        }
    else
        defineVariable(env,newSymbol(EnvironmentName),0);
    }
