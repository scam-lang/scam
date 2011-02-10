#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "cell.h"
#include "types.h"
#include "util.h"

#define STACKSIZE (4096 * 4)

int MemorySpot;
int MemorySize =  4 * 128 * 8192; 

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

    MemorySpot = 1;

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
    uninitializedSymbol  = newSymbol("UNINITIALIZED");
    }

int
getMemorySize()
   {
   return MemorySize;
   }

int
cons(int a,int b)
    {
    CELL *spot;

    spot = the_cars + MemorySpot;
    spot->type = CONS;
    spot->ival = a;
    spot->line = LineNumber;
    spot->file = FileIndex;

    the_cdrs[MemorySpot] = b;

    ++MemorySpot;

    return MemorySpot - 1;
    }

int
newString(char *s)
    {
    int start;
    int length = strlen(s);

    start = MemorySpot;

    while (*s != 0)
        {
        type(MemorySpot) = STRING;
        ival(MemorySpot) = *s;
        count(MemorySpot) = length;
        line(MemorySpot) = LineNumber;
        file(MemorySpot) = FileIndex;

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

    the_cdrs[MemorySpot] = 0;

    ++MemorySpot;

    return start;
    }

int
newSymbol(char *s)
    {
    int index = findSymbol(s);
    int result = cons(0,0);
    type(result) = SYMBOL;
    ival(result) = index;
    return result;
    }

int
newInteger(int i)
    {
    int result = cons(0,0);
    type(result) = INTEGER;
    ival(result) = i;
    return result;
    }

int
newReal(double r)
    {
    int result = cons(0,0);
    type(result) = REAL;
    rval(result) = r;
    return result;
    }

int
newPunctuation(char *t)
    {
    int result = cons(0,0);
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
