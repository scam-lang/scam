#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "cell.h"
#include "types.h"

#define STACKSIZE (4096 * 4)

int MemorySpot;
int MemorySize =  4 * 128 * 8192; 

void (*save)(int,char *);
int (*restore)(char *);
int  savedRegisters[STACKSIZE];
int saveCount = 0;

#if CHECK_SAVES
char *savedRegisterNames[STACKSIZE];
static void saveCheck(int,char *);
static int restoreCheck(char *);
#else
static void saveNoCheck(int,char *);
static int restoreNoCheck(char *);
#endif

CELL *the_cars;
CELL *new_cars;
int *the_cdrs;
int *new_cdrs;

int LineNumber;
int FileIndex;
int Indent;
int Indenting = 1;

int MaxSymbols = 100;

static int symbol_equal(void *,void *);
static int SymbolCount;
static int SymbolsIncrement = 100;

void
memoryInit(int memsize)
    {
    SymbolTable = (char **) malloc(MaxSymbols * sizeof(char *));
    if (SymbolTable == 0)
        Fatal(OUT_OF_MEMORY, "could not allocate Symbol table\n");

    if (memsize > 0) MemorySize = memsize;

    the_cars = (CELL *) malloc(sizeof(CELL) * MemorySize);
    if (the_cars == 0)
        Fatal(OUT_OF_MEMORY, "could not allocate code segment (cars)\n");
    new_cars = (CELL *) malloc(sizeof(CELL) * MemorySize);
    if (new_cars == 0)
        Fatal(OUT_OF_MEMORY, "could not allocate code segment\n");

    the_cdrs = (int *) malloc(sizeof(int) * MemorySize);
    if (the_cdrs == 0)
        Fatal(OUT_OF_MEMORY, "could not allocate code segment (cdrs)\n");
    new_cdrs = (int *) malloc(sizeof(int) * MemorySize);
    if (new_cdrs == 0)
        Fatal(OUT_OF_MEMORY, "could not allocate code segment\n");

    MemorySpot = 1;
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

    ensureMemory(length + 1); /* extra cell for null character */

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
isThisSymbol(int a,char *name)
    {
    return type(a) == SYMBOL && strcmp(SymbolTable[ival(a)],name) == 0;
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
    int result;
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
        Fatal(OUT_OF_MEMORY,"Symbol Table is full\n");

    SymbolTable[SymbolCount++] = dup;

    return SymbolCount - 1;
    }
