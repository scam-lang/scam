#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
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
    int closure;
    
    assureMemory("defineFunction",CLOSURE_CELLS + DEFINE_CELLS,&name,&parameters,&body,&env,0);

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
        return throw(exceptionSymbol,"can only define SYMBOLS, not type %s",type(first));
    }

/* (addSymbol name init env) */

static int
addSymbol(int args)
    {
    return defineIdentifier(car(args),cadr(args),caddr(args));
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

/* (or # a $b) */

static int
or(int args)
    {
    if (cadr(args) == trueSymbol) return trueSymbol;

    return makeThunk(caddr(args),car(args));
    }

/* (and # a $b) */

static int
and(int args)
    {
    if (cadr(args) == falseSymbol) return falseSymbol;
    //debug("and",caddr(args));

    return makeThunk(caddr(args),car(args));
    }

static int
not(int args)
    {
    if (sameSymbol(car(args),trueSymbol)) return falseSymbol;
    if (sameSymbol(car(args),falseSymbol)) return trueSymbol;
    return throw(exceptionSymbol,"not: non-boolean");
    }

/* (< a b) */

static int
isLessThan(int args)
    {
    int a,b;
    char *aType,*bType;

    a = car(args);
    b = cadr(args);
    aType = type(a);
    bType = type(b);

    if (aType == INTEGER && bType == INTEGER)
        return scamBoolean(ival(a) < ival(b));
    else if (aType == INTEGER && bType == REAL)
        return scamBoolean(ival(a) < rval(b));
    else if (aType == REAL && bType == INTEGER)
        return scamBoolean(rval(a) < ival(b));
    else if (aType == REAL && bType == REAL)
        return scamBoolean(rval(a) < rval(b));
    else
        return throw(exceptionSymbol,"wrong types for '<': %s and %s",aType,bType);
    }

/* (<= a b) */

static int
isLessThanOrEqualTo(int args)
    {
    int a,b;
    char *aType,*bType;

    a = car(args);
    b = cadr(args);
    aType = type(a);
    bType = type(b);

    if (aType == INTEGER && bType == INTEGER)
        return scamBoolean(ival(a) <= ival(b));
    else if (aType == INTEGER && bType == REAL)
        return scamBoolean(ival(a) <= rval(b));
    else if (aType == REAL && bType == INTEGER)
        return scamBoolean(rval(a) <= ival(b));
    else if (aType == REAL && bType == REAL)
        return scamBoolean(rval(a) <= rval(b));
    else
        return throw(exceptionSymbol,"wrong types for '<=': %s and %s",aType,bType);
    }

/* (= a b) */

static int
isNumericEqualTo(int args)
    {
    int a,b;
    char *aType,*bType;

    a = car(args);
    b = cadr(args);
    aType = type(a);
    bType = type(b);

    if (aType == INTEGER && bType == INTEGER)
        return scamBoolean(ival(a) == ival(b));
    else if (aType == INTEGER && bType == REAL)
        return scamBoolean(ival(a) == rval(b));
    else if (aType == REAL && bType == INTEGER)
        return scamBoolean(rval(a) == ival(b));
    else if (aType == REAL && bType == REAL)
        return scamBoolean(rval(a) == rval(b));
    else
        return throw(exceptionSymbol,"wrong types for '=': %s and %s",aType,bType);
    }

/* (> a b) */

static int
isGreaterThan(int args)
    {
    int a,b;
    char *aType,*bType;

    a = car(args);
    b = cadr(args);
    aType = type(a);
    bType = type(b);

    if (aType == INTEGER && bType == INTEGER)
        return scamBoolean(ival(a) > ival(b));
    else if (aType == INTEGER && bType == REAL)
        return scamBoolean(ival(a) > rval(b));
    else if (aType == REAL && bType == INTEGER)
        return scamBoolean(rval(a) > ival(b));
    else if (aType == REAL && bType == REAL)
        return scamBoolean(rval(a) > rval(b));
    else
        return throw(exceptionSymbol,"wrong types for '>': %s and %s",aType,bType);
    }

/* (>= a b) */

static int
isGreaterThanOrEqualTo(int args)
    {
    int a,b;
    char *aType,*bType;

    a = car(args);
    b = cadr(args);
    aType = type(a);
    bType = type(b);

    if (aType == INTEGER && bType == INTEGER)
        return scamBoolean(ival(a) >= ival(b));
    else if (aType == INTEGER && bType == REAL)
        return scamBoolean(ival(a) >= rval(b));
    else if (aType == REAL && bType == INTEGER)
        return scamBoolean(rval(a) >= ival(b));
    else if (aType == REAL && bType == REAL)
        return scamBoolean(rval(a) >= rval(b));
    else
        return throw(exceptionSymbol,"wrong types for '>=': %s and %s",aType,bType);
    }

/* (eq? a b) */

static int
isEq(int args)
    {
    int a = car(args);
    int b = cadr(args);

    if (type(a) != type(b)) return falseSymbol;

    if (type(a) == INTEGER) return scamBoolean(ival(a) == ival(b));
    if (type(a) == REAL) return scamBoolean(ival(a) == ival(b));
    if (type(a) == SYMBOL) return scamBoolean(ival(a) == ival(b));

    return scamBoolean(a == b);
    }

/* (neq? a b) */

static int
isNotEq(int args)
    {
    int a = car(args);
    int b = cadr(args);

    if (type(a) != type(b)) return trueSymbol;

    if (type(a) == INTEGER) return scamBoolean(ival(a) != ival(b));
    if (type(a) == REAL) return scamBoolean(ival(a) != ival(b));
    if (type(a) == SYMBOL) return scamBoolean(ival(a) != ival(b));

    return scamBoolean(a != b);
    }

/* (+ a b) */

static int
plus(int args)
    {
    int a,b;
    char *aType,*bType;

    a = car(args);
    b = cadr(args);
    aType = type(a);
    bType = type(b);

    if (aType == INTEGER && bType == INTEGER)
        return newInteger(ival(a) + ival(b));
    else if (aType == INTEGER && bType == REAL)
        return newReal(ival(a) + rval(b));
    else if (aType == REAL && bType == INTEGER)
        return newReal(rval(a) + ival(b));
    else if (aType == REAL && bType == REAL)
        return newReal(rval(a) + rval(b));
    else
        return throw(exceptionSymbol,"wrong types for '+': %s and %s",aType,bType);
    }

/* (- a b) */

static int
minus(int args)
    {
    int a,b;
    char *aType,*bType;

    a = car(args);
    b = cadr(args);
    aType = type(a);
    bType = type(b);

    if (aType == INTEGER && bType == INTEGER)
        return newInteger(ival(a) - ival(b));
    else if (aType == INTEGER && bType == REAL)
        return newReal(ival(a) - rval(b));
    else if (aType == REAL && bType == INTEGER)
        return newReal(rval(a) - ival(b));
    else if (aType == REAL && bType == REAL)
        return newReal(rval(a) - rval(b));
    else
        return throw(exceptionSymbol,"wrong types for '-': %s and %s",aType,bType);
    }

/* (* a b) */

static int
times(int args)
    {
    int a,b;
    char *aType,*bType;

    a = car(args);
    b = cadr(args);
    aType = type(a);
    bType = type(b);

    if (aType == INTEGER && bType == INTEGER)
        return newInteger(ival(a) * ival(b));
    else if (aType == INTEGER && bType == REAL)
        return newReal(ival(a) * rval(b));
    else if (aType == REAL && bType == INTEGER)
        return newReal(rval(a) * ival(b));
    else if (aType == REAL && bType == REAL)
        return newReal(rval(a) * rval(b));
    else
        return throw(exceptionSymbol,"wrong types for '*': %s and %s",aType,bType);
    }

/* (/ a b) */

static int
divides(int args)
    {
    int a,b;
    char *aType,*bType;

    a = car(args);
    b = cadr(args);
    aType = type(a);
    bType = type(b);

    if (aType == INTEGER && bType == INTEGER)
        return newInteger(ival(a) / ival(b));
    else if (aType == INTEGER && bType == REAL)
        return newReal(ival(a) / rval(b));
    else if (aType == REAL && bType == INTEGER)
        return newReal(rval(a) / ival(b));
    else if (aType == REAL && bType == REAL)
        return newReal(rval(a) / rval(b));
    else
        return throw(exceptionSymbol,"wrong types for '/': %s and %s",aType,bType);
    }

/* (% a b) */

static int
mod(int args)
    {
    int a,b;
    char *aType,*bType;

    a = car(args);
    b = cadr(args);
    aType = type(a);
    bType = type(b);

    if (aType == INTEGER && bType == INTEGER)
        return newInteger(ival(a) % ival(b));
    else
        return throw(exceptionSymbol,"wrong types for '%': %s and %s",aType,bType);
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

/* (print @) */

static int
print(int args)
    {
    int last = 0;
    FILE *port = OpenPorts[CurrentOutputIndex];

    args = car(args);

    while (args != 0)
        {
        last = car(args);
        pp(port,car(args));
        args = cdr(args);
        }

    return last;
    }

static int
println(int args)
    {
    FILE *port = OpenPorts[CurrentOutputIndex];
    int result = print(args);
    fprintf(port,"\n");
    return result;
    }

/* (if # test $then $) */

static int
iif(int args)
    {
    //debug("if test",cadr(args));

    if (sameSymbol(cadr(args),trueSymbol))
        {
        //printf("if test is true\n");
        //debug("then",cadr(args));
        return makeThunk(caddr(args),car(args));
        }
    else
        {
        //printf("if test is false\n");
        int otherwise = cadddr(args);
        if (otherwise != 0)
            {
            //debug("else",cadr(args));
            return makeThunk(car(otherwise),car(args));
            }
        else
            return 0;
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
    if (type(car(args)) != CONS)
        return throw(exceptionSymbol,
            "attempt to set the car of type %s",type(car(args)));

    caar(args) = cadr(args);
    return cadr(args);
    }

/* (set-cdr! spot value) */

static int
setCdr(int args)
    {
    if (type(car(args)) != CONS)
        return throw(exceptionSymbol,
            "attempt to set the car of type %s",type(car(args)));

    cdar(args) = cadr(args);
    return cadr(args);
    }

/* (set! id value # @) */

static int
set(int args)
    {
    int id = car(args);
    int result;
    
    //printf("in set!...");
    if (type(id) != SYMBOL)
        return throw(exceptionSymbol,
            "set! identifier resolved to type %s, not SYMBOL",type(id));

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
    int env;
    int result;
    
    //printf("in assign...");
    if (type(id) == SYMBOL)
        return set(args);

    if (type(id) != CONS && sameSymbol(car(id),dotSymbol))
        return throw(exceptionSymbol,
            "first argument to assign was not a symbol or a dot operation");

    env = cadr(id);
    id = caddr(id);

    push(id);
    push(args);
    env = eval(env,caddr(args));
    args = pop();
    id = pop();

    rethrow(env,0);

    result = setVariableValue(id,cadr(args),env);

    return result;
    }


/* (get id & @) */

static int 
get(int args)
    {
    int id = car(args);

    //printf("in get...");
    if (type(id) != SYMBOL)
        return throw(exceptionSymbol,
            "get variable argument resolved to type %s, not SYMBOL",type(id));

    if (caddr(args) == 0)
        return getVariableValue(id,cadr(args));
    else
        return getVariableValue(id,car(caddr(args)));
    }

static int
force(int args)
    {
    if (!isThunk(car(args)))
        return throw(exceptionSymbol,"cannot force type %s",type(car(args)));

    return car(args);
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
    char *t = type(car(args));

    if (t != CONS && t != STRING && t != ARRAY)
        return throw(exceptionSymbol,
            "attempt to take car of type %s",type(car(args)));

    return car(car(args));
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
addOpenPort(FILE *fp,int portType)
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
        return throw(exceptionSymbol,"too many ports open at once");
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
                "%s is not a valid argument to setPort",
                SymbolTable[ival(target)]);
        }
    else if (type(target) == CONS && sameSymbol(car(target),inputPortSymbol))
        {
        old = CurrentInputIndex;
        CurrentInputIndex = ival(cadr(target));
        return ucons(inputPortSymbol,ucons(newInteger(old),0));
        }
    else if (type(target) == CONS && sameSymbol(car(target),outputPortSymbol))
        {
        old = CurrentOutputIndex;
        CurrentOutputIndex = ival(cadr(target));
        return ucons(outputPortSymbol,ucons(newInteger(old),0));
        }

    return throw(exceptionSymbol,
        "setPort given a non-port as argument: %s",type(target));
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
cclose(int args)
    {
    int target,index;

    target = car(args);
    debug("port is",target);

    if (type(target) == CONS && sameSymbol(car(target),inputPortSymbol))
        {
        index = ival(cadr(target));
        if (index == 0)
            {
            return throw(exceptionSymbol,"attempt to close stdin");
            }
        if (index >= MaxPorts)
            {
            return throw(exceptionSymbol,
                "attempt to close a non-existent port: %d",index);
            }
        if (OpenPorts[index] == 0)
            {
            return throw(exceptionSymbol,
                "attempt to close an unopened port: %d",index);
            }
        fclose(OpenPorts[index]);
        OpenPorts[index] = 0;
        if (CurrentInputIndex == index) CurrentInputIndex = 0;
        }
    else if (type(target) == CONS && sameSymbol(car(target),outputPortSymbol))
        {
        index = ival(cadr(target));
        if (index == 1)
            {
            return throw(exceptionSymbol,"attempt to close stdout");
            }
        if (index >= MaxPorts)
            {
            return throw(exceptionSymbol,
                "attempt to close a non-existent port: %d",index);
            }
        if (OpenPorts[index] == 0)
            {
            return throw(exceptionSymbol,
                "attempt to close an unopened port: %d",index);
            }
        fclose(OpenPorts[index]);
        OpenPorts[index] = 0;
        if (CurrentOutputIndex == index) CurrentOutputIndex = 1;
        }
    else
        return throw(exceptionSymbol,
            "bad type passed to close: %s", type(target));

    return trueSymbol;
    }

static int
readChar(int args)
    {
    int ch;
    char buffer[3];
    FILE *fp;

    fp = OpenPorts[CurrentInputIndex];

    if (fp == 0)
        return throw(exceptionSymbol,"attempt to read a character from a closed port");

    if (feof(fp))
        return throw(exceptionSymbol,"attempt to read a character at end of input");

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
readInt(int args)
    {
    int i;
    FILE *fp;

    fp = OpenPorts[CurrentInputIndex];

    if (fp == 0)
        return throw(exceptionSymbol,"attempt to read an integer from a closed port");

    if (feof(fp))
        return throw(exceptionSymbol,"attempt to read an integer at end of input");

    i = 0;
    fscanf(fp," %d",&i);
    return newInteger(i);
    }

static int
readReal(int args)
    {
    double r;
    FILE *fp;

    fp = OpenPorts[CurrentInputIndex];

    if (fp == 0)
        return throw(exceptionSymbol,"attempt to read a real from a closed port");

    if (feof(fp))
        return throw(exceptionSymbol,"attempt to read a real at end of input");

    r = 0;
    fscanf(fp," %lf",&r);
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

    if (fp == 0)
        return throw(exceptionSymbol,"attempt to read a string from a closed port");

    if (feof(fp))
        return throw(exceptionSymbol,"attempt to read a string at end of input");

    skipWhiteSpace(fp);

    ch = fgetc(fp);
    if (ch != '\"')
        {
        ungetc(ch,fp);
        return throw(exceptionSymbol,
            "found <%c> instead of double quote at the start of a string",ch);
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
                return throw(exceptionSymbol,"attempt to read a string at end of input");
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
            return throw(exceptionSymbol,"attempt to read a very long string failed");
        }

    buffer[index] = '\0';

    if (ch != '\"')
        return throw(exceptionSymbol,"attempt to read an unterminated string");

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

    if (fp == 0)
        return throw(exceptionSymbol,"attempt to read a token from a closed port");

    if (feof(fp))
        return throw(exceptionSymbol,"attempt to read a token at end of input");

    skipWhiteSpace(fp);

    index = 0;
    while ((ch = fgetc(fp)) && ch != EOF && !isspace(ch))
        {
        buffer[index++] = ch;
        if (index == sizeof(buffer) - 1)
            return throw(exceptionSymbol,"attempt to read a very long token failed");
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
            "readWhile argument should be STRING, not type %s",type(a));
        }

    fp = OpenPorts[CurrentInputIndex];

    if (fp == 0)
        return throw(exceptionSymbol,"attempt to read characters from a closed port");

    if (feof(fp))
        return throw(exceptionSymbol,"attempt to read characters at end of input");

    cellStringTr(target,sizeof(target),a);

    index = 0;
    while ((ch = fgetc(fp)) && ch != EOF && strchr(target,ch) != 0)
        {
        buffer[index++] = ch;
        if (index == sizeof(buffer) - 1)
            return throw(exceptionSymbol,"attempt to read a very long token failed");
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
            "readWhile argument should be STRING, not type %s",type(a));
        }

    fp = OpenPorts[CurrentInputIndex];

    if (fp == 0)
        return throw(exceptionSymbol,"attempt to read characters from a closed port");

    if (feof(fp))
        return throw(exceptionSymbol,"attempt to read characters at end of input");

    cellStringTr(target,sizeof(target),a);

    index = 0;
    while ((ch = fgetc(fp)) && ch != EOF && strchr(target,ch) == 0)
        {
        buffer[index++] = ch;
        if (index == sizeof(buffer) - 1)
            return throw(exceptionSymbol,"attempt to read a very long token failed");
        }

    buffer[index] = '\0';

    ungetc(ch,fp);

    //printf("token is <%s>\n", buffer);

    result = newString(buffer);

    return result;
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

    if (fp == 0)
        return throw(exceptionSymbol,"attempt to read a line from a closed port");

    if (feof(fp))
        return throw(exceptionSymbol,"attempt to read a line at end of input");

    index = 0;
    while ((ch = fgetc(fp)) && ch != EOF && ch != '\n')
        {
        buffer[index++] = ch;
        if (index == sizeof(buffer) - 1)
            return throw(exceptionSymbol,"attempt to read a very long line failed");
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
                return throw(exceptionSymbol,"file %s cannot be opened for reading",buffer);
            result = addOpenPort(fp,inputPortSymbol);
            }
        else if (ival(mode) == writeIndex)
            {
            char buffer[256];
            FILE *fp = fopen(cellString(buffer,sizeof(buffer),target),"w");
            //printf("buffer is %s\n",buffer);
            if (fp == 0)
                return throw(exceptionSymbol,"file %s cannot be opened for writing",buffer);
            //printf("file opened successfully\n");
            result = addOpenPort(fp,outputPortSymbol);
            }
        else if (ival(mode) == appendIndex)
            {
            char buffer[256];
            FILE *fp = fopen(cellString(buffer,sizeof(buffer),target),"a");
            if (fp == 0)
                return throw(exceptionSymbol,"file %s cannot be opened for appending",buffer);
            result = addOpenPort(fp,outputPortSymbol);
            }
        else 
            {
            return throw(exceptionSymbol,"unknown open mode :%s, "
                "(should be 'read, 'write, or 'append)",
                SymbolTable[ival(mode)]);
            }
        }
    else
        {
        return throw(exceptionSymbol,"unknown mode type: %s, "
            "(should be 'read, 'write, or 'append)",
            type(mode));
        }

    debug("new port",result);
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
        return throw(exceptionSymbol,"cannot take the length of type %s",
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
            "negative indices (%d) are not allowed",index);

    if (kind == ARRAY)
        limit = count(supply);
    else if (kind == STRING || kind == CONS)
        limit = length(supply);
    else
        return throw(exceptionSymbol,"cannont index into type %s",kind);

    if (index >= limit)
        return throw(exceptionSymbol,"index (%d) is too large",index);

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
            "negative indices (%d) are not allowed",index);

    if (kind == ARRAY)
        limit = count(supply);
    else if (kind == STRING || kind == CONS)
        limit = length(supply);
    else
        return throw(exceptionSymbol,"cannont index into type %s",kind);

    if (index >= limit)
        return throw(exceptionSymbol,"index (%d) is too large",index);

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
        car(supply) = value;
        return car(supply);
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
        return throw(exceptionSymbol,"wrong number of arguments to throw");
    }

/* string manipulations */

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
            "prefix: first argument should be STRING, not %s",
            type(a));
        }

    if (type(b) != INTEGER)
        {
        return throw(exceptionSymbol,
            "prefix: second argument should be INTEGER, not %s",
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
            "suffix: first argument should be STRING, not %s",
            type(a));
        }

    if (type(b) != INTEGER)
        {
        return throw(exceptionSymbol,
            "suffix: second argument should be INTEGER, not %s",
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
            "stringWhile: both arguments should be STRING, not %s and %s",
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
            "stringUntil: both arguments should be STRING, not %s and %s",
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
            "substring: first argument is type %s (should be %s)",
            type(needle),STRING);
        }

    if (type(haystack) != STRING)
        {
        return throw(exceptionSymbol,
            "substring: second argument is %s (should be %s)",
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
            "trim: argument is type %s (should be %s)",
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
        newSymbol("setElement!"),
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
        0,
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readInt;
    b = makeBuiltIn(env,
        newSymbol("readInt"),
        0,
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readReal;
    b = makeBuiltIn(env,
        newSymbol("readReal"),
        0,
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readString;
    b = makeBuiltIn(env,
        newSymbol("readString"),
        0,
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readToken;
    b = makeBuiltIn(env,
        newSymbol("readToken"),
        0,
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = readLine;
    b = makeBuiltIn(env,
        newSymbol("readLine"),
        0,
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
        0,
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = getInputPort;
    b = makeBuiltIn(env,
        newSymbol("getInputPort"),
        0,
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
        0,
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
        ucons(newSymbol("a"),
            ucons(newSymbol("b"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = minus;
    b = makeBuiltIn(env,
        newSymbol("-"),
        ucons(newSymbol("a"),
            ucons(newSymbol("b"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = times;
    b = makeBuiltIn(env,
        newSymbol("*"),
        ucons(newSymbol("a"),
            ucons(newSymbol("b"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = divides;
    b = makeBuiltIn(env,
        newSymbol("/"),
        ucons(newSymbol("a"),
            ucons(newSymbol("b"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = mod;
    b = makeBuiltIn(env,
        newSymbol("%"),
        ucons(newSymbol("a"),
            ucons(newSymbol("b"),0)),
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

    BuiltIns[count] = and;
    b = makeBuiltIn(env,
        newSymbol("and"),
        ucons(sharpSymbol,
            ucons(newSymbol("a"),
                ucons(newSymbol("$b"),0))),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = or;
    b = makeBuiltIn(env,
        newSymbol("or"),
        ucons(sharpSymbol,
            ucons(newSymbol("a"),
                ucons(newSymbol("$b"),0))),
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
        ucons(newSymbol("a"),
            ucons(newSymbol("b"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isLessThanOrEqualTo;
    b = makeBuiltIn(env,
        newSymbol("<="),
        ucons(newSymbol("a"),
            ucons(newSymbol("b"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isGreaterThan;
    b = makeBuiltIn(env,
        newSymbol(">"),
        ucons(newSymbol("a"),
            ucons(newSymbol("b"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isGreaterThanOrEqualTo;
    b = makeBuiltIn(env,
        newSymbol(">="),
        ucons(newSymbol("a"),
            ucons(newSymbol("b"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isEq;
    b = makeBuiltIn(env,
        newSymbol("eq?"),
        ucons(newSymbol("a"),
            ucons(newSymbol("b"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isEq;
    b = makeBuiltIn(env,
        newSymbol("=="),
        ucons(newSymbol("a"),
            ucons(newSymbol("b"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isNotEq;
    b = makeBuiltIn(env,
        newSymbol("neq?"),
        ucons(newSymbol("a"),
            ucons(newSymbol("b"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isNumericEqualTo;
    b = makeBuiltIn(env,
        newSymbol("="),
        ucons(newSymbol("a"),
            ucons(newSymbol("b"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = isNotEq;
    b = makeBuiltIn(env,
        newSymbol("!="),
        ucons(newSymbol("a"),
            ucons(newSymbol("b"),0)),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = get;
    b = makeBuiltIn(env,
        newSymbol("get"),
        ucons(newSymbol("id"),
            ucons(sharpSymbol,
                ucons(atSymbol,0))),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = set;
    b = makeBuiltIn(env,
        newSymbol("set!"),
        ucons(newSymbol("id"),
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
