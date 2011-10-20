#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "cell.h"
#include "types.h"
#include "util.h"
#include "pp.h"

#define STACKSIZE (4096 * 4)

int MemorySpot;
int MemorySize =  2 * 2 * 2 * 2 * 4 * 16 * 32 * 64; 
int StackPtr = 0;
int StackSize = STACKSIZE;
int *Stack;

int zero;
int one;
int contextSymbol;
int codeSymbol;
int thisSymbol;
int parametersSymbol;
int thunkSymbol;
int nameSymbol;
int envSymbol;
int closureSymbol;
int builtInSymbol;
int constructorSymbol;
int objectSymbol;
int typeSymbol;
int labelSymbol;
int valueSymbol;
int traceSymbol;
int throwSymbol;
int errorSymbol;
int quoteSymbol;
int anonymousSymbol;
int lambdaSymbol;
int dollarSymbol;
int atSymbol;
int uninitializedSymbol;
int errorSymbol;
int beginSymbol;
int scopeSymbol;
int sharpSymbol;
int trueSymbol;
int falseSymbol;
int trueWordSymbol;
int falseWordSymbol;
int backquoteSymbol;
int commaSymbol;
int inputPortSymbol;
int outputPortSymbol;
int eofSymbol;
int elseSymbol;
int nilSymbol;
int nullSymbol;
int fileSymbol;
int lineSymbol;
int messageSymbol;
int spacerSymbol;
int defineSymbol;
int exceptionSymbol;
int mathExceptionSymbol;
int lexicalExceptionSymbol;
int syntaxExceptionSymbol;
int nonFunctionSymbol;
int returnSymbol;
int levelSymbol;
int eqSymbol;
int dotSymbol;
int DOTSymbol;
int assignSymbol;
int undefinedVariableSymbol;
int uninitializedVariableSymbol;

int readIndex;
int writeIndex;
int appendIndex;
int stdinIndex;
int stdoutIndex;

CELL *the_cars;
CELL *new_cars;
int *the_cdrs;
int *new_cdrs;

int MaxSymbols = 100;
char **SymbolTable;
int SymbolCount;
static int SymbolsIncrement = 100;
static int gccount = 0;

static int rootBottom;

void
memoryInit(int memsize)
    {
    Stack = (int *) malloc(sizeof(int) * StackSize);
    if (Stack == 0)
        Fatal("could not allocate Stack\n");

    SymbolTable = (char **) malloc(MaxSymbols * sizeof(char *));
    if (SymbolTable == 0)
        Fatal("could not allocate Symbol table\n");

    if (memsize > 0) MemorySize = memsize;

    the_cars = (CELL *) malloc(sizeof(CELL) * MemorySize);
    if (the_cars == 0)
        Fatal("could not allocate code segment (cars)\n");
    new_cars = (CELL *) malloc(sizeof(CELL) * MemorySize);
    if (new_cars == 0)
        Fatal("could not allocate code segment (new cars)\n");

    the_cdrs = (int *) malloc(sizeof(int) * MemorySize);
    if (the_cdrs == 0)
        Fatal("could not allocate code segment (cdrs)\n");
    new_cdrs = (int *) malloc(sizeof(int) * MemorySize);
    if (new_cdrs == 0)
        Fatal("could not allocate code segment (new cdrs)\n");

    /* nil has to be the first symbol */

    newSymbol("nil");

    assert(MemorySpot == 1);

    zero = newInteger(0);
    one = newInteger(1);

    labelSymbol          = newSymbol("__label");
    contextSymbol        = newSymbol("__context");
    levelSymbol          = newSymbol("__level");
    constructorSymbol    = newSymbol("__constructor");
    codeSymbol           = newSymbol("code");
    thisSymbol           = newSymbol("this");
    parametersSymbol     = newSymbol("parameters");
    thunkSymbol          = newSymbol("thunk");
    nameSymbol           = newSymbol("name");
    envSymbol            = newSymbol("environment");
    closureSymbol        = newSymbol("closure");
    builtInSymbol        = newSymbol("builtIn");
    objectSymbol         = newSymbol("object");
    typeSymbol           = newSymbol("type");
    valueSymbol          = newSymbol("value");
    traceSymbol          = newSymbol("trace");
    throwSymbol          = newSymbol("throw");
    errorSymbol          = newSymbol("error");
    quoteSymbol          = newSymbol("quote");
    anonymousSymbol      = newSymbol("anonymous");
    lambdaSymbol         = newSymbol("lambda");
    dollarSymbol         = newSymbol("$");
    atSymbol             = newSymbol("@");
    sharpSymbol          = newSymbol("#");
    uninitializedSymbol  = newSymbol(":UNINITIALIZED:");
    errorSymbol          = newSymbol("error");
    beginSymbol          = newSymbol("begin");
    scopeSymbol          = newSymbol("scope");
    trueSymbol           = newSymbol("#t");
    falseSymbol          = newSymbol("#f");
    trueWordSymbol       = newSymbol("true");
    falseWordSymbol      = newSymbol("false");
    backquoteSymbol      = newSymbol("backquote");
    commaSymbol          = newSymbol("comma");
    inputPortSymbol      = newSymbol("inputPort");
    outputPortSymbol     = newSymbol("outputPort");
    eofSymbol            = newSymbol("EOF");
    elseSymbol           = newSymbol("else");
    nilSymbol            = newSymbol("nil");
    nullSymbol           = newSymbol("null");
    fileSymbol           = newSymbol("file");
    lineSymbol           = newSymbol("line");
    messageSymbol        = newSymbol("message");
    spacerSymbol         = newSymbol(" SPACER");
    defineSymbol         = newSymbol("define");
    exceptionSymbol      = newSymbol("generalException");
    mathExceptionSymbol  = newSymbol("mathException");
    lexicalExceptionSymbol = newSymbol("lexicalException");
    syntaxExceptionSymbol = newSymbol("syntaxException");
    nonFunctionSymbol    = newSymbol("nonFunction");
    returnSymbol         = newSymbol("return");
    eqSymbol             = newSymbol("=");
    dotSymbol            = newSymbol(".");
    DOTSymbol            = newSymbol("dot");
    assignSymbol         = newSymbol("assign");
    undefinedVariableSymbol = newSymbol("undefinedVariable");
    uninitializedVariableSymbol = newSymbol("uninitializedVariable");

    readIndex            = findSymbol("read");
    writeIndex           = findSymbol("write");
    appendIndex          = findSymbol("append");
    stdinIndex           = findSymbol("stdin");
    stdoutIndex          = findSymbol("stdout");

    rootBottom = MemorySpot;
    }

int
getMemorySize()
   {
   return MemorySize;
   }

int
cons(int a,int b)
    {
    assureMemory("cons",1,&a,&b,0);

    //assert(line(a) != 0);
    //assert(file(a) != 0);
    return uconsfl(a,b,file(a),line(a));
    }

int
ucons(int a,int b)
    {
    CELL *spot;

    spot = the_cars + MemorySpot;
    spot->type = CONS;
    spot->ival = a;
    spot->line = line(a);
    spot->file = file(a);
    spot->transferred = 0;

    the_cdrs[MemorySpot] = b;

    ++MemorySpot;

    return MemorySpot - 1;
    }

int
uconsfl(int a,int b,int fileIndex,int lineNumber)
    {
    int c;
    
    c = ucons(a,b);
    file(c) =  fileIndex;
    line(c) = lineNumber;

    return c;
    }

/* append is destructive! */

int
append(int list1,int list2)
    {
    int start = list1;
    while (cdr(list1) != 0)
        list1 = cdr(list1);

    cdr(list1) = list2;

    return start;
    }

int
newString(char *s)
    {
    int start;
    int length = strlen(s);

    assureMemory("newString",length,0);

    start = MemorySpot;

    while (*s != 0)
        {
        type(MemorySpot) = STRING;
        ival(MemorySpot) = *s;
        count(MemorySpot) = length;
        line(MemorySpot) = 0;
        file(MemorySpot) = 0;
        transferred(MemorySpot) = 0;

        cdr(MemorySpot) = MemorySpot + 1;

        --length;
        ++s;
        ++MemorySpot;
        }
    cdr(MemorySpot - 1) = 0;

    return start;
    }

int
newSymbol(char *s)
    {
    int index = findSymbol(s);
    int result;

    assureMemory("newSymbol",1,0);

    result = ucons(0,0);
    type(result) = SYMBOL;
    ival(result) = index;
    return result;
    }

int
newSymbolFromIndex(int index)
    {
    int result;

    assureMemory("newSymbolFromIndex",1,0);

    result = ucons(0,0);
    type(result) = SYMBOL;
    ival(result) = index;
    return result;
    }

int
newInteger(int i)
    {
    int result;

    assureMemory("newInteger",1,0);

    result = ucons(0,0);
    type(result) = INTEGER;
    ival(result) = i;
    return result;
    }

int
newReal(double r)
    {
    int result;

    assureMemory("newReal",1,0);

    result = ucons(0,0);
    type(result) = REAL;
    rval(result) = r;
    return result;
    }

int
newPunctuation(char *t)
    {
    int result;

    assureMemory("newPuncuation",1,0);

    result = ucons(0,0);
    type(result) = t;
    return result;
    }

int
cellStrCmp(int a,int b)
    {
    while (ival(a) != 0 && ival(b) != 0)
        {
        if (ival(a) != ival(b)) return ival(a) - ival(b);
        a = cdr(a);
        b = cdr(b);
        }

    if (ival(a) != 0 || ival(b) != 0) return ival(a) - ival(b);

    return ival(a) - ival(b);
    }

char *
cellString(char *buffer,int size, int s)
    {
    int i = 0;
    char *target;
    static char store[4096];

    //printf("in cellString...\n");
    if (buffer == 0)
        {
        target = store;
        size = sizeof(store);
        }
    else
        target = buffer;

    while (i < size - 1 && s != 0)
        {
        target[i] = (char) ival(s);
        //printf("target[%d] is %c\n",i,target[i]);
        s = cdr(s);
        ++i;
        }

    target[i] = '\0';

    return target;
    }

char *
cellStringTr(char *buffer,int size, int s)
    {
    int i = 0;
    char *target;
    static char store[4096];

    //printf("in cellStringTr...\n");
    if (buffer == 0)
        {
        target = store;
        size = sizeof(store);
        }
    else
        target = buffer;

    while (i < size - 2 && s != 0)
        {
        if (ival(s) == '\\')
            {
            s = cdr(s);
            if (ival(s) == 'n')
                target[i] = '\n';
            else if (ival(s) == 'r')
                target[i] = '\r';
            else if (ival(s) == 't')
                target[i] = '\t';
            else
                target[i] = (char) ival(s);
            }
        else
            target[i] = (char) ival(s);
        //printf("target[%d] is %c\n",i,target[i]);
        s = cdr(s);
        ++i;
        }

    target[i] = '\0';

    return target;
    }

int
findSymbol(char *s)
    {
    int i;
    char *dup;

    for (i = 0; i < SymbolCount; ++i)
        if (strcmp(s,SymbolTable[i]) == 0)
            return i;

    if (SymbolCount >= MaxSymbols)
        {
        MaxSymbols += SymbolsIncrement;
        SymbolTable =
            (char **) realloc(SymbolTable,sizeof(char *) * MaxSymbols);
        }

    dup = strdup(s);
    if (SymbolTable == 0 || dup == 0)
        Fatal("Symbol Table is full\n");

    SymbolTable[SymbolCount++] = dup;

    return SymbolCount - 1;
    }

int
pop()
    {
    int temp;
    assert(StackPtr > 0);
    temp = Stack[--StackPtr];
    //debug("pop ",temp);
    return temp;
    }

void push(int i)
    {
    //debug("push",i);
    if (StackPtr >= StackSize)
        {
        printf("stack ptr is %d, stack size is %d\n",StackPtr,StackSize);
        assert(StackPtr < StackSize);
        }
    Stack[StackPtr++] = i;
    //printf("stack ptr now is %d\n",StackPtr);
    }

static int
transferBackbone(int old,int limit)
    {
    int size,start;
    char *t = type(old);

    //printf("   transferred tag is %d\n",transferred(old));
    if (t != STRING && t != ARRAY)
        {
        //debug("transferring",old);
        assert(transferred(old) == 0);
        new_cars[limit] = the_cars[old];
        new_cdrs[limit] = the_cdrs[old];
        //printf("placing it at %d\n",limit);
        //getchar();
        transferred(old) = 1;
        cdr(old) = limit;
        return limit + 1;
        }

    /* must be array or string */
    //printf("backbone type is %s\n",t);

    /* first, find the start of the contiguous region */
    //printf("array or string start was %d\n",old);
    //debug("before",old);
    while (!transferred(old-1) && type(old-1) == t && cdr(old-1) == old)
        {
        //printf("cdr(old-1) is %d, old is %d\n",cdr(old-1),old);
        --old;
        //debug("string now is",old);
        }
    //printf("array or string start now is %d\n",old);
    //debug("after",old);
    size = count(old);
    //getchar();

    //debug("transferring",old);
    //printf("placing it at %d\n",limit);
    //printf("size is %d\n",size);
    start = limit;
    //if (t == STRING) debug("before",old);
    while (size != 0)
        {
        //printf("   transferred tag at %d is %d\n",old,transferred(old));
        assert(transferred(old) == 0);
        new_cars[limit] = the_cars[old];
        new_cdrs[limit] = the_cdrs[old];
        transferred(old) = 1;
        cdr(old) = limit;
        ++limit;
        ++old;
        --size;
        }
    //if (t == STRING)
        //{
        //printf("last character is %c\n",new_cars[limit-1].ival);
        //printf("last cdr is %d\n",new_cdrs[limit-1]);
        //printf("last old character is %c\n",the_cars[old-1].ival);
        //printf("last old cdr is %d\n",the_cdrs[old-1]);
        //}
    //printf("limit is %d\n",limit);
    //getchar();
    //if (t == STRING)
    //    {
    //    printf("after:  ");
    //    while (new_cars[start].count >= 1)
    //         printf("%c",new_cars[start++].ival);
    //    printf("\n");
    //    }
        
    //debug("after",start);

    return limit;
    }

static int
transfer(int limit)
    {
    int spot = 1;
    while (spot < limit)
        {
        int old;

        /* only need to transfer over conses, strings, and arrays */

        //printf("spot %d: ",spot);
        if (new_cars[spot].type == CONS)
            {
            /* transfer over the cdr, if necessary */

            //printf("found a cons!\n");
            old = new_cdrs[spot];
            if (!transferred(old))
                {
                //debug("transferring",old);
                limit = transferBackbone(old,limit);
                }
            //else
                //debug("TRANSFERRED ",old);

            /* update the cdr to the transferred locaiion */

            new_cdrs[spot] = cdr(old);

            /* transfer over the car, if necessary */

            old = new_cars[spot].ival;
            if (!transferred(old))
                {
                //debug("transferring",old);
                limit = transferBackbone(old,limit);
                }
            //else
                //debug("TRANSFERRED ",old);

            /* update the car to the transferred locaiion */

            new_cars[spot].ival = cdr(old);

            //getchar();
            }
        else if (new_cars[spot].type == STRING)
            {
            /* cdrs should already be transferred */

            //printf("found a string!\n");
            //old = new_cdrs[spot];
            //if (!transferred(old))
                //{
                //debug("calling backbone",old);
                //assert(0);
                //limit = transferBackbone(old,limit);
                //}
            //else
                //debug("TRANSFERRED ",old);

            /* update the cdr to the transferred locaiion */

            //new_cdrs[spot] = cdr(old);
            }
        else if (new_cars[spot].type == ARRAY)
            {
            /* cdr should already be transferred */

            //printf("found an array!\n");
            //old = new_cdrs[spot];
            //if (!transferred(old))
            //    {
            //    //debug("transferring",old);
            //    assert(0);
            //    limit = transferBackbone(old,limit);
            //    }
            //else
                //debug("TRANSFERRED ",old);

            /* update the cdr to the transferred locaiion */

            //new_cdrs[spot] = cdr(old);

            /* transfer over the car, if necessary */

            //printf("transferring over array cars\n");

            old = new_cars[spot].ival;
            if (!transferred(old))
                {
                //debug("transferring",old);
                limit = transferBackbone(old,limit);
                }
            //else
                //debug("TRANSFERRED ",old);

            /* update the car to the transferred locaiion */

            new_cars[spot].ival = cdr(old);

            //getchar();
            }
        //else
            //printf("TRANSFERRED: %s\n",type(spot));
        ++spot;
        }
    //printf("new memory spot is %d\n",spot);
    return spot;
    }


             
void 
gc()
    {
    int i,spot;
    int *temp_cdrs;
    CELL *temp_cars;

    //for (i = 0; i < StackPtr; ++i)
    //    {
    //    printf("Stack[%d]:Location %d:",i,Stack[i]);
    //    debug("",Stack[i]);
    //    }

    /* transfer over symbols */

    for (i = 0; i < rootBottom; ++i)
        {
        new_cars[i] = the_cars[i];
        new_cdrs[i] = the_cdrs[i];
        transferred(i) = 1;
        cdr(i) = i;
        }

    spot = i;

    /* transfer over the root list */

    for (i = 0; i < StackPtr; ++i)
        {
        if (!transferred(Stack[i]))
            {
            new_cars[spot] = the_cars[Stack[i]];
            new_cdrs[spot] = the_cdrs[Stack[i]];
            transferred(Stack[i]) = 1;
            cdr(Stack[i]) = spot;
            ++spot;
            }
        /* point the stack item to its new location */
        Stack[i] = cdr(Stack[i]);
        }

    MemorySpot = transfer(spot);

    /* swap the new and old memory */

    temp_cars = the_cars;
    the_cars = new_cars;
    new_cars = temp_cars;

    temp_cdrs = the_cdrs;
    the_cdrs = new_cdrs;
    new_cdrs = temp_cdrs;

    //for (i = 0; i < StackPtr; ++i)
    //    {
    //    printf("Stack[%d]:Location %d:",i,Stack[i]);
    //    debug("",Stack[i]);
    //    }

    printf("gc:%d, %d cells\n",++gccount,MemorySpot);

    //for (i = 0; i < MemorySize; ++i)
    //    new_cars[i].type = PAST;

    //for (i = MemorySpot; i < MemorySize; ++i)
    //    the_cars[i].type = FUTURE;
    }


void
assureMemory(char *tag,int needed, int *item, ...)
    {
    va_list ap;
    int i;
    int *store[10];
    int storeSize = sizeof(store) / sizeof(int *);
    int storePtr = 0;

    va_start(ap, item);

    if (MemorySpot + needed >= MemorySize)
        {
        /* save items */
        //printf("gc called from %s\n",tag);
        
        while (item != 0)
            {
            //debug("pushing",*item);
            push(*item);
            assert(storePtr < storeSize);
            store[storePtr++] = item;
            item = va_arg(ap,int *);
            }

        gc();

        if (MemorySpot + needed >= MemorySize)
            Fatal("gc failed: out of memory\n");

        /* restore items (reverse order) */

        for (i = storePtr - 1;i >= 0;--i)
            {
            *(store[i]) = pop();
            //debug("popping",*(store[i]));
            }

        //getchar();
        }
    }

int
length(int items)
    {
    int total = 0;
    while (items != 0)
        {
        items = cdr(items);
        total += 1;
        }

    return total;
    }
