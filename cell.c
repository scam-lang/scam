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
int MemorySize =  2 * 128 * 8192; 
int StackPtr = 0;
int Stack[1000];
int StackSize = sizeof(Stack) / sizeof(int);

int zero;
int one;
int contextSymbol;
int codeSymbol;
int dynamicContextSymbol;
int thisSymbol;
int parametersSymbol;
int thunkSymbol;
int nameSymbol;
int closureSymbol;
int builtInSymbol;
int constructorSymbol;
int objectSymbol;
int typeSymbol;
int valueSymbol;
int traceSymbol;
int throwSymbol;
int quoteSymbol;
int anonymousSymbol;
int lambdaSymbol;
int dollarSymbol;
int atSymbol;
int uninitializedSymbol;
int errorSymbol;
int beginSymbol;
int sharpSymbol;
int trueSymbol;
int falseSymbol;
int backquoteSymbol;
int commaSymbol;

CELL *the_cars;
CELL *new_cars;
int *the_cdrs;
int *new_cdrs;

int LineNumber;
int FileIndex;

int MaxSymbols = 100;
char **SymbolTable;
int SymbolCount;
static int SymbolsIncrement = 100;

int rootList;
static int rootBottom;

void
memoryInit(int memsize)
    {
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

    rootList = 0;

    zero = newInteger(0);
    one = newInteger(1);

    contextSymbol        = newSymbol("context");
    codeSymbol           = newSymbol("code");
    dynamicContextSymbol = newSymbol("dynamicContext");
    thisSymbol           = newSymbol("this");
    parametersSymbol     = newSymbol("parameters");
    thunkSymbol          = newSymbol("thunk");
    nameSymbol           = newSymbol("name");
    closureSymbol        = newSymbol("closure");
    builtInSymbol        = newSymbol("builtIn");
    constructorSymbol    = newSymbol("constructor");
    objectSymbol         = newSymbol("object");
    typeSymbol           = newSymbol("type");
    valueSymbol          = newSymbol("value");
    traceSymbol          = newSymbol("trace");
    throwSymbol          = newSymbol("throw");
    quoteSymbol          = newSymbol("quote");
    anonymousSymbol      = newSymbol("anonymous");
    lambdaSymbol         = newSymbol("lambda");
    dollarSymbol         = newSymbol("$");
    atSymbol             = newSymbol("@");
    sharpSymbol          = newSymbol("#");
    uninitializedSymbol  = newSymbol(":UNINITIALIZED:");
    errorSymbol          = newSymbol("error");
    beginSymbol          = newSymbol("begin");
    trueSymbol           = newSymbol("#t");
    falseSymbol          = newSymbol("#f");
    backquoteSymbol      = newSymbol("backquote");
    commaSymbol          = newSymbol("comma");

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
    assureMemory(1,&a,&b,0);

    return ucons(a,b);
    }

int
ucons(int a,int b)
    {
    CELL *spot;

    spot = the_cars + MemorySpot;
    spot->type = CONS;
    spot->ival = a;
    spot->line = LineNumber;
    spot->file = FileIndex;
    spot->transferred = 0;

    the_cdrs[MemorySpot] = b;

    ++MemorySpot;

    return MemorySpot - 1;
    }


int
newString(char *s)
    {
    int start;
    int length = strlen(s);

    assureMemory(length + 1,0);

    start = MemorySpot;

    while (*s != 0)
        {
        type(MemorySpot) = STRING;
        ival(MemorySpot) = *s;
        count(MemorySpot) = length;
        line(MemorySpot) = LineNumber;
        file(MemorySpot) = FileIndex;
        transferred(MemorySpot) = 0;

        cdr(MemorySpot) = MemorySpot + 1;

        --length;
        ++s;
        ++MemorySpot;
        }

    the_cars[MemorySpot].type = STRING;
    the_cars[MemorySpot].ival = '\0';
    the_cars[MemorySpot].count = 0;
    the_cars[MemorySpot].line = LineNumber;
    the_cars[MemorySpot].file = FileIndex;
    the_cars[MemorySpot].transferred = 0;

    the_cdrs[MemorySpot] = 0;

    ++MemorySpot;

    return start;
    }

int
newSymbol(char *s)
    {
    int index = findSymbol(s);
    int result;

    assureMemory(1,0);

    result = ucons(0,0);
    type(result) = SYMBOL;
    ival(result) = index;
    return result;
    }

int
newInteger(int i)
    {
    int result;

    assureMemory(1,0);

    result = ucons(0,0);
    type(result) = INTEGER;
    ival(result) = i;
    return result;
    }

int
newReal(double r)
    {
    int result;

    assureMemory(1,0);

    result = ucons(0,0);
    type(result) = REAL;
    rval(result) = r;
    return result;
    }

int
newPunctuation(char *t)
    {
    int result;

    assureMemory(1,0);

    result = ucons(0,0);
    type(result) = t;
    return result;
    }

int
cellStrcmp(int a,int b)
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
    assert(StackPtr < StackSize);
    Stack[StackPtr++] = i;
    }

static int
transfer(int limit)
    {
    int spot = 1;
    while (spot < limit)
        {
        /* only need to transfer over conses */

        printf("spot %d\n",spot);
        if (new_cars[spot].type == CONS)
            {
            int old;

            /* transfer over the car, if necessary */

            old = new_cars[spot].ival;
            if (!transferred(old))
                {
                debug("transferring",old);
                new_cars[limit] = the_cars[old];
                new_cdrs[limit] = the_cdrs[old];
                transferred(old) = 1;
                cdr(old) = limit;
                ++limit;
                }
            else
                debug("TRANSFERRED ",old);

            /* update the car to the transferred locaiion */

            new_cars[spot].ival = cdr(old);

            /* transfer over the cdr, if necessary */

            old = new_cdrs[spot];
            if (!transferred(old))
                {
                debug("transferring",old);
                new_cars[limit] = the_cars[old];
                new_cdrs[limit] = the_cdrs[old];
                transferred(old) = 1;
                cdr(old) = limit;
                ++limit;
                }
            else
                debug("TRANSFERRED ",old);

            /* update the car to the transferred locaiion */

            new_cdrs[spot] = cdr(old);

            getchar();

            }
        ++spot;
        }
    printf("new memory spot is %d\n",spot);
    return spot;
    }

void 
gc()
    {
    int i,spot;
    int *temp_cdrs;
    CELL *temp_cars;

    ppObject(stdout,Stack[0],0);
    for (i = 0; i < StackPtr; ++i)
        {
        debug("root list was",Stack[i]);
        printf("    at location %d\n",Stack[i]);
        }

    printf("MemorySpot is %d\n",MemorySpot);

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
        debug("root list after symbol transfer was",Stack[i]);
        printf("    at location %d\n",Stack[i]);
        if (!transferred(Stack[i]))
            {
            new_cars[spot] = the_cars[Stack[i]];
            new_cdrs[spot] = the_cdrs[Stack[i]];
            transferred(Stack[i]) = 1;
            cdr(Stack[i]) = spot;
            }
        /* update the stack to hold the new location */
        Stack[i] = cdr(Stack[i]);
        printf("    new location %d\n",Stack[i]);
        ++spot;
        }

    //transfer(rootList);

    ppObject(stdout,Stack[0],0);
    MemorySpot = transfer(spot);

    /* swap the new and old memory */

    temp_cars = the_cars;
    the_cars = new_cars;
    new_cars = temp_cars;

    temp_cdrs = the_cdrs;
    the_cdrs = new_cdrs;
    new_cdrs = temp_cdrs;

    for (i = 0; i < StackPtr; ++i)
        {
        printf("    new location %d\n",Stack[i]);
        debug("root list now is",Stack[i]);
        }

    getchar();

    for (i = 0; i < MemorySpot; ++i)
        {
        printf("%d: ",i);
        debug("new",i);
        }

    getchar();
    }


void
assureMemory(int needed, int *item, ...)
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
        
        while (item != 0)
            {
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
            *(store[i]) = pop();
        }
    }
