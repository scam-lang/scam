#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "types.h"
#include "env.h"
#include "cell.h"
#include "prim.h"
#include "eval.h"
#include "pp.h"
#include "util.h"

PRIM BuiltIns[1000];
FILE *OpenPorts[20];
int MaxPorts = sizeof(OpenPorts) / sizeof(FILE *);
int CurrentInputIndex;
int CurrentOutputIndex;

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
rtimes(int args,int accum)
    {
    while (args != 0)
        {
        int arg = car(args);
        char *t = type(arg);
        if (t == INTEGER)
            rval(accum) *= ival(arg);
        else if (t == REAL)
            rval(accum) *= rval(arg);
        else
            Fatal("wrong type for multiplication: %s\n",type(car(args)));
        args = cdr(args);
        }

    return accum;
    }

static int
itimes(int args,int accum)
    {
    while (args != 0)
        {
        char *t = type(car(args));

        if (t == REAL)
            {
            type(accum) = REAL;
            rval(accum) = ival(accum);
            return rtimes(args,accum);
            }
        else if (t == INTEGER)
            ival(accum) *= ival(car(args));
        else
            {
            Fatal("wrong type for multiplication: %s\n",type(car(args)));
            }
            
        args = cdr(args);
        }

    //debug("iplus",accum);
    return accum;
    }

/* (+ @) */

static int
times(int args)
    {
    char *t;

    args = car(args);

    if (args == 0) return zero;

    assureMemory("times",1,&args,0);

    t = type(car(args));

    if (t == INTEGER) return itimes(args,newInteger(0));
    if (t == REAL) return rtimes(args,newReal(0));
    
    return Fatal("wrong type for multiplication: %s\n",type(car(args)));
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
            Fatal("wrong type for addition: %s\n",type(car(args)));
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
            Fatal("wrong type for addition: %s\n",type(car(args)));
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
    
    return Fatal("wrong type for addition: %s\n",type(car(args)));
    }

static int
rdivides(int args,int accum)
    {
    while (args != 0)
        {
        int arg = car(args);
        char *t = type(arg);
        if (t == INTEGER)
            rval(accum) /= ival(arg);
        else if (t == REAL)
            rval(accum) /= rval(arg);
        else
            Fatal("wrong type for division: %s\n",type(car(args)));
        args = cdr(args);
        }

    return accum;
    }

static int
idivides(int args,int accum)
    {
    while (args != 0)
        {
        char *t = type(car(args));

        if (t == REAL)
            {
            type(accum) = REAL;
            rval(accum) = ival(accum);
            return rdivides(args,accum);
            }
        else if (t == INTEGER)
            ival(accum) /= ival(car(args));
        else
            Fatal("wrong type for division: %s\n",type(car(args)));
            
        args = cdr(args);
        }

    return accum;
    }

/* (/ @) */

static int
divides(int args)
    {
    char *t;
    int accum;

    args = car(args);

    if (args == 0) return zero;

    assureMemory("divides",1,&args,0);

    t = type(car(args));

    if (t == INTEGER && cdr(args) == 0)
        {
        return newInteger(1 / ival(car(args)));
        }
    else if (t == INTEGER)
        {
        accum = newInteger(ival(car(args)));
        return idivides(cdr(args),accum);
        }
    else if (t == REAL && cdr(args) == 0)
        {
        return newReal(1 / rval(car(args)));
        }
    else if (t == REAL)
        {
        accum = newReal(rval(car(args)));
        return rdivides(cdr(args),accum);
        }
    
    return Fatal("wrong type for division: %s\n",t);
    }

static int
mod(int args)
    {
    char *t;
    int accum;
    
    args = car(args);

    if (args == 0) return zero;
    if (cdr(args) == 0) return zero;

    t = type(car(args));

    if (t != INTEGER)
        return Fatal("wrong type for remainder: %s\n",type(car(args)));

    assureMemory("remainder",1,&args,0);

    accum = newInteger(ival(car(args)));
    args = cdr(args);

    while (args != 0)
        {
        t = type(car(args));

        if (t == INTEGER)
            ival(accum) %= ival(car(args));
        else
            Fatal("wrong type for remainder: %s\n",type(car(args)));
            
        args = cdr(args);
        }

    return accum;
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
            Fatal("wrong type for subtraction: %s\n",type(car(args)));
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
            Fatal("wrong type for subtraction: %s\n",type(car(args)));
            
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

    if (t == INTEGER && cdr(args) == 0)
        {
        return newInteger(0 - ival(car(args)));
        }
    else if (t == INTEGER)
        {
        accum = newInteger(ival(car(args)));
        return iminus(cdr(args),accum);
        }
    else if (t == REAL && cdr(args) == 0)
        {
        return newReal(0 - rval(car(args)));
        }
    else if (t == REAL)
        {
        accum = newReal(rval(car(args)));
        return rminus(cdr(args),accum);
        }
    
    return Fatal("wrong type for subtraction: %s\n",t);
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

/* (car item) */

static int
ccar(int args)
    {
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
addOpenPort(FILE *fp,int portType,int target)
    {
    int i;
    int maxPorts = sizeof(OpenPorts) / sizeof(FILE *);

    for (i = 2; i < maxPorts; ++i)
        {
        //printf("port[%d] is %p\n",i,OpenPorts[i]);
        if (OpenPorts[i] == 0)
            break;
        }

    if (i == maxPorts)
        {
        return Fatal("fileOpenError","too many ports open");
        }

    OpenPorts[i] = fp;

    assureMemory("addOpenPort",3,0);

    return ucons(portType,ucons(newInteger(i),0));
    }

int
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
        }
    else if (type(target) == CONS && sameSymbol(car(target),inputPortSymbol))
        {
        old = CurrentInputIndex;
        CurrentInputIndex = ival(target);
        return ucons(inputPortSymbol,ucons(newInteger(old),0));
        }
    else if (type(target) == CONS && sameSymbol(car(target),outputPortSymbol))
        {
        old = CurrentOutputIndex;
        CurrentOutputIndex = ival(target);
        return ucons(outputPortSymbol,ucons(newInteger(old),0));
        }

    return Fatal("argumentTypeError",
        "setPort given a non-port as argument: %s",
        type(target));
    }

int
getInputPort(int args)
    {
    assureMemory("getInputPort",3,0);
    return ucons(inputPortSymbol,ucons(newInteger(CurrentInputIndex),0));
    }

int
getOutputPort(int args)
    {
    assureMemory("getOutputPort",3,0);
    return ucons(outputPortSymbol,ucons(newInteger(CurrentInputIndex),0));
    }

static int
cclose(int args)
    {
    int target,index;

    target = car(args);
    index = ival(target);

    if (type(target) == CONS && sameSymbol(car(target),inputPortSymbol))
        {
        if (index == 0)
            {
            return Fatal("illegal attempt to close stdin");
            }
        if (index >= MaxPorts)
            {
            return Fatal("illegal attempt to a non-existent port");
            }
        if (OpenPorts[index] == 0)
            {
            return Fatal("illegal attempt to close an unopened port");
            }
        fclose(OpenPorts[index]);
        OpenPorts[index] = 0;
        if (CurrentInputIndex == index) CurrentInputIndex = 0;
        }
    else if (type(target) == CONS && sameSymbol(car(target),outputPortSymbol))
        {
        if (index == 1)
            {
            return Fatal("illegal attempt to close stdout");
            }
        if (index >= MaxPorts)
            {
            return Fatal("illegal attempt to a non-existent port");
            }
        if (OpenPorts[index] == 0)
            {
            return Fatal("illegal attempt to close an unopened port");
            }
        fclose(OpenPorts[index]);
        OpenPorts[index] = 0;
        if (CurrentOutputIndex == index) CurrentOutputIndex = 1;
        }
    else
        return Fatal("bad type passed to close: %s", type(target));

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
        return Fatal("attempt to read a character from a closed port");

    ch = fgetc(fp);

    if (feof(fp))
        return Fatal("attempt to read a character at end of input");

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
        return Fatal("attempt to read an integer from a closed port");

    if (feof(fp))
        return Fatal("attempt to read an integer at end of input");

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
        return Fatal("attempt to read a real from a closed port");

    if (feof(fp))
        return Fatal("attempt to read a real at end of input");

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

    skipWhiteSpace(fp);

    ch = fgetc(fp);
    if (ch != '\"')
        {
        ungetc(ch,fp);
        return Fatal("reading a string: found <%c> instead of double quote",ch);
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
                return Fatal("reading a string: unexpected end of input");
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
            return Fatal("reading a string: string too large");
        }

    buffer[index] = '\0';

    if (ch != '\"')
        return Fatal("reading a string: unterminated string");

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

    skipWhiteSpace(fp);

    index = 0;
    while ((ch = fgetc(fp)) && ch != EOF && !isspace(ch))
        {
        buffer[index++] = ch;
        if (index == sizeof(buffer) - 1)
            return Fatal("reading a token: token too large");
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
        return Fatal("readWhile: argument should be STRING, not %s",type(a));
        }

    fp = OpenPorts[CurrentInputIndex];

    cellStringTr(target,sizeof(target),a);

    index = 0;
    while ((ch = fgetc(fp)) && ch != EOF && strchr(target,ch) != 0)
        {
        buffer[index++] = ch;
        if (index == sizeof(buffer) - 1)
            return Fatal("readWhile: token too long");
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
        return Fatal("argumentTypeError",
            "readWhile: argument should be STRING, not %s",
            type(a));
        }

    fp = OpenPorts[CurrentInputIndex];

    cellStringTr(target,sizeof(target),a);

    index = 0;
    while ((ch = fgetc(fp)) && ch != EOF && strchr(target,ch) == 0)
        {
        buffer[index++] = ch;
        if (index == sizeof(buffer) - 1)
            return Fatal("IOError", "readWhile: token too long");
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

    index = 0;
    while ((ch = fgetc(fp)) && ch != EOF && ch != '\n')
        {
        buffer[index++] = ch;
        if (index == sizeof(buffer) - 1)
            return Fatal("IOError","reading a line: line too long");
        }

    buffer[index] = '\0';

    //printf("token is <%s>\n", buffer);

    result = newString(buffer);

    return result;
    }

static int
eeof(int args)
    {
    FILE *fp = OpenPorts[CurrentInputIndex];
    if (fp == 0 || feof(fp))
        return trueSymbol;
    else
        return falseSymbol;
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
                return Fatal("file %s cannot be opened for reading",buffer);
            result = addOpenPort(fp,inputPortSymbol,target);
            }
        else if (ival(mode) == writeIndex)
            {
            char buffer[256];
            FILE *fp = fopen(cellString(buffer,sizeof(buffer),target),"w");
            //printf("buffer is %s\n",buffer);
            if (fp == 0)
                return Fatal("file %s cannot be opened for writing",buffer);
            //printf("file opened successfully\n");
            result = addOpenPort(fp,outputPortSymbol,target);
            }
        else if (ival(mode) == appendIndex)
            {
            char buffer[256];
            FILE *fp = fopen(cellString(buffer,sizeof(buffer),target),"a");
            if (fp == 0)
                return Fatal("file %s cannot be opened for appending",buffer);
            result = addOpenPort(fp,outputPortSymbol,target);
            }
        else 
            {
            return Fatal("unknown open mode :%s, "
                "(should be 'read, 'write, or 'append)",
                SymbolTable[ival(mode)]);
            }
        }
    else
        {
        return Fatal("unknown mode type: %s, "
            "(should be 'read, 'write, or 'append)",
            type(mode));
        }

    return result;
    }

void
loadBuiltIns(int env)
    {
    int b;
    int count = 0;

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

    BuiltIns[count] = cclose;
    b = makeBuiltIn(env,
        newSymbol("close"),
        ucons(newSymbol("port"),0),
        newInteger(count));
    defineVariable(env,closure_name(b),b);
    ++count;

    BuiltIns[count] = eeof;
    b = makeBuiltIn(env,
        newSymbol("eof?"),
        0,
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

    OpenPorts[0] = stdin;
    OpenPorts[1] = stdout;
    CurrentInputIndex = 0;
    CurrentOutputIndex = 1;
    }
