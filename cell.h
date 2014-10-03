
/*
 *  Main Author : John C. Lusth
 *  Barely Authors : Jeffrey Robinson, Gabriel Loewen
 *  Last Edit : May 4, 2014
 *
 *  The layout of this document is such:
 *      Includes
 *      Structs
 *      extern global variables
 *      external functions
 *      #defines
 *      extern symbol variables
 */


#ifndef __CELL_H__
#define __CELL_H__

#include "util.h"   // Fatal
#include "thread.h" // pthread_getspecific,P/V functions, and key

typedef struct shadowobj
{
    int addr;

    char *type;            /* The cell type (SYMBOL,INTEGER,...)*/

    short file;            /* Index into SymbolTable for filename */
    short line;            /* If applicable is line it appeared on in the source */
    int count;             /* length of ARRAY or STRING */

    int ival;              /* INTEGER->value, STRING->character, CONS->car */
    double rval;           /* Holds real value*/
    int (*fval)(int,int);  /* Pointer to pimitive */

    int cdr;              /* For a CONS,STRING, or ARRAY this is the next element */ 

    char* fileChanged;
    int   lineChanged;

} S_CELL;

/* the basic cell structure */
typedef struct cellobj
    {
    char **type;            /* The cell type (SYMBOL,INTEGER,...)*/

    short *line;            /* If applicable is line it appeared on in the source */
    short *file;            /* Index into SymbolTable for filename */
    int *count;             /* length of ARRAY or STRING */

    int *ival;              /* INTEGER->value, STRING->character, CONS->car */
    double *rval;           /* Holds real value*/
    int (**fval)(int,int);  /* Pointer to pimitive */

    char *status;           /* Reserved by GC */
    int *fwd;               /* Reserved by GC */

    int *cdr;              /* For a CONS,STRING, or ARRAY this is the next element */ 

    /* TODO : Remove eventually */
    int *creator;              /* Thread who created me */
    int *lastEditor;          /* Which thread created this cell */
    char **lastFile;
    int  *lastLine;

    } CELL;

/* TODO : MOVE THIS DOWN */
#define creator(X)          (THE_CARS.creator[X])
#define lastEditor(X)       (THE_CARS.lastEditor[X])
#define lastFile(X)         (THE_CARS.lastFile[X])
#define lastLine(X)         (THE_CARS.lastLine[X])

#define setEditor(X,V)      (lastEditor(X)=(V))
#define setLastFile(X,V)    (lastFile(X)=(V))
#define setLastLine(X,V)    (lastLine(X)=(V))

/* Global Data-structure sizes */

extern int StackSize;       /* How big the stack can grow */
extern int HeapBottom;      /* The last Symbol position added duringinit */
extern int MemorySize;      /* Heap size, can be split into two heaps */

/**** Global arrays  *****/

/* Symbol Table */
extern char **SymbolTable;  /* Has Symbols (Source code, added by programmer) */


/* Stack  */
extern int *Stack[];        /* Used to store items when we might do a GC */
extern int StackSpot[];     /* Each thread has it's own stack spot */

/* 
 *  Shadow Stack - A shadow copy of the stack is held.  It is only 
 *      modified in the PUSH, POP, and REPLACE routines.  If we 
 *      do not use a PUSH,POP, or REPLACE as we should this is used 
 *      to detect that.
 */
/* TODO : Remove this eventually : Moved into thread.h -> T_DATA  */
extern S_CELL *ShadowStack[];  /* Copy of valid stack */
extern int ShadowSpot[];    /* Each thread has it's own stack */


/* Heap */
/* TODO : Maybe each thread can have it's own stack, have idea. */
extern CELL theCars[];
extern int MemorySpot[];


/* Free List */
extern int FreeCount[]; /* Some GCs keeps a list of freeded cells */

/**** Public functions ****/

/* Startup and shutdown methods for scam */
extern void scamInit(int);
extern void scamShutdown();

extern void memoryInit();
extern void memoryShutdown();

extern void printStack(void);

#define cons(a,b) t_cons(a,b,__FILE__,__LINE__)

/* Helper functions */
extern int t_cons(int,int,char*,int);
extern int cons2(int,int);
extern int consfl(int,int,int,int);
extern int append(int,int);
extern int length(int);
extern int allocateContiguous(char *,int);

/* lexing helpers */
extern int newString(char *);
extern int newStringUnsafe(char *);
extern int newSymbol(char *);
extern int newSymbolUnsafe(char *);
extern int newInteger(int);
extern int newIntegerUnsafe(int);
extern int newReal(double);
extern int newPunctuation(char *);

extern int findSymbol(char *);

extern char *cellString(char *,int,int);
extern char *cellStringTr(char *,int,int);

/* Memory GC routines */
extern void ensureMemory(char *,int,int,int *,...);
extern void ensureContiguousMemory(char *,int,int,int *,...);
extern void stopAndCopy(void);
extern void reclaim(void);

extern void print_stack(void);

/* Powers of two */
#define CONSTANT 4
#define UPDATED 2
#define MARKED 1
#define UNMARKED 0

/* Index into each each data-structure for that thread.
   Useful for changes that can be made lateri 
   (ie. multiple heaps vs a single heap).
*/
#define INDEX 0

#define THE_CARS        (theCars[INDEX])

#define FREE_LIST       (FreeList[INDEX])
#define STACK           (Stack[THREAD_ID])

#define MEMORY_SPOT     (MemorySpot[INDEX])
#define NEW_MEM_SPOT    (newMemorySpot[INDEX])
#define FREE_COUNT      (FreeCount[INDEX])
#define STACK_SPOT      (StackSpot[THREAD_ID])

#define NEW_CARS        (newCars[INDEX])

/* TODO : Future work, possibility of each thread having it's own heap
 *          FROM is the heap
 *          ADDR is the address in the heap.
 *          LOCAL is whether an index is in the local heap
 *
 *      Possible #defines
 *          type(a)    (theCars[FROM(a)][ADDR(a)].type)
 *
 *          line(a)    (theCars[FROM(a)][ADDR(a)].line)
 *          file(a)    (theCars[FROM(a)][ADDR(a)].file)
 *          count(a)   (theCars[FROM(a)][ADDR(a)].count)
 *
 *          ival(a)    (theCars[FROM(a)][ADDR(a)].ival)
 *          rval(a)    (theCars[FROM(a)][ADDR(a)].rval)
 *          fval(a)    (theCars[FROM(a)][ADDR(a)].fval)
 *
 *          status(a)   (theCars[FROM(a)][ADDR(a)].status)
 */
#define FROM(a)         ((a>>27)&0x3F)
#define ADDR(a)         (a & 0x7FFFFFF)
#define LOCAL(a)        (FROM(A)==THREAD_ID)

/* Mutators */
#define type(a)         (THE_CARS.type[a])
#define settype(A,V)                \
do                                  \
    {                               \
    type(A)=V;                      \
    if(StackDebugging)              \
        {                           \
        setEditor(A,THREAD_ID);     \
        setLastFile(A,__FILE__);    \
        setLastLine(A,__LINE__);    \
        if(creator(A) != THREAD_ID)  \
            {                           \
            printf("Thread %d, touching Thread %d's type\n",THREAD_ID,creator(A));\
            printf("File : %s, Line : %d\n", __FILE__, __LINE__);\
            }                           \
        }                           \
    }while(0)

#define count(a)        (THE_CARS.count[a])
#define setcount(a,v)   (count(a)=v)

#define file(a)         (THE_CARS.file[a])
#define setfile(a,v)    (file(a)=v)

#define line(a)         (THE_CARS.line[a])
#define setline(a,v)    (line(a)=v)

#define ival(a)         (THE_CARS.ival[a])

#define setival(A,V)                \
do                                  \
    {                               \
    ival(A)=V;                      \
    if(StackDebugging)              \
        {                           \
        setEditor(A,THREAD_ID);     \
        setLastFile(A,__FILE__);    \
        setLastLine(A,__LINE__);    \
        if(creator(A) != THREAD_ID)  \
            {                           \
            printf("Thread %d, touching Thread %d's ival\n",THREAD_ID,creator(A));\
            printf("File : %s, Line : %d\n", __FILE__, __LINE__);\
            }                           \
        }                           \
    }while(0)

#define rval(a)         (THE_CARS.rval[a])
#define setrval(a,v)                    \
do                                      \
    {                                   \
    int spot = a;                       \
    rval(spot) = v;                     \
    if(StackDebugging)                  \
        {                               \
        setEditor(spot,THREAD_ID);     \
        setLastFile(spot,__FILE__);    \
        setLastLine(spot,__LINE__);    \
        if(creator(spot) != THREAD_ID)  \
            {                           \
            printf("Thread %d, touching Thread %d's rval\n",THREAD_ID,creator(spot));\
            printf("File : %s, Line : %d\n", __FILE__, __LINE__);\
            }                           \
        }                               \
    }while(0)

#define fval(a)         (THE_CARS.fval[a])
#define setfval(a,v)    (fval(a)=v)

#define fwd(a)          (THE_CARS.fwd[a])
#define setfwd(a,v)     (fwd(a)=v)

#define status(a)       (THE_CARS.status[a]) 
#define setstatus(a,v)  (status(a)=v)

#define name(a)         (SymbolTable[ival(a)])
#define transferred(a)  (status(a))
#define ismarked(a)     (status(a) == MARKED)
#define isupdated(a)    (status(a) == UPDATED)
#define filename(a)     (SymbolTable[THE_CARS.file[a]])

#define car(a)          (THE_CARS.ival[a])
#define setcar(A,V)     setival(A,V)

#define cdr(a)          (THE_CARS.cdr[a])
#define setcdr(a,v)                         \
do                                          \
    {                                       \
    int spot = a;                           \
    cdr(spot)=v;                            \
    if(StackDebugging)                      \
        {                                   \
        setEditor(a,THREAD_ID);             \
        setLastFile(spot,__FILE__);            \
        setLastLine(spot,__LINE__);            \
        if(creator(spot) != THREAD_ID)      \
            {                               \
            printf("Thread %d, touching Thread %d's cdr\n",THREAD_ID,creator(spot));\
            printf("File : %s, Line : %d\n", __FILE__, __LINE__);\
            }                               \
        }                                   \
    }while(0)

#define cdar(a)         (cdr(car(a)))
#define setcdar(a,v)    (cdar(a)=v)
#define cddr(a)         (cdr(cdr(a)))
#define cdddr(a)        (cdr(cddr(a)))
#define cddddr(a)       (cdr(cdddr(a)))
#define cadr(a)         (car(cdr(a)))
#define caddr(a)        (car(cddr(a)))
#define cadddr(a)       (car(cdddr(a)))
#define caddddr(a)      (car(cddddr(a)))


#define SameSymbol(a,b) (type(a) == SYMBOL && ival(a) == ival(b))

#define newtype(a)      (NEW_CARS.type[a])
#define newcar(a)       (NEW_CARS.ival[a])
#define newline(a)      (NEW_CARS.line[a])
#define newfile(a)      (NEW_CARS.file[a])
#define newcount(a)     (NEW_CARS.count[a])
#define newival(a)      (NEW_CARS.ival[a])
#define newrval(a)      (NEW_CARS.rval[a])
#define newfval(a)      (NEW_CARS.fval[a])
#define newstatus(a)    (NEW_CARS.status[a])
#define newcdr(a)       (NEW_CARS.cdr[a])


/*  Try to ensure there are 'size' contigouos cells on the heap.  If not then perform 
 *   a garbage collection.  We also make sure to push on any items that we might 
 *   modify
 */
#define ENSURE_CONTIGUOUS_MEMORY(size,item,...)                 \
    {                                                           \
    if (MEMORY_SPOT + size >= MemorySize)                       \
        {                                                       \
        if(Debugging || StackDebugging)                         \
            printf("gc %d from line %s,%d\n",GCCount,__FILE__,__LINE__);   \
        ensureContiguousMemory(__FILE__,__LINE__,size,item,##__VA_ARGS__);        \
        }                                                       \
    }


/*  Try to ensure there are 'size' cells on the heap.  If not then perform 
 *   a garbage collection.  We also make sure to push on any items that we
 *   might modify
 */
#define ENSURE_MEMORY(size,item,...)                            \
    {                                                           \
    if (FREE_COUNT < size && (MEMORY_SPOT + size) >= MemorySize)\
        {                                                       \
        if(Debugging || StackDebugging)                         \
            printf("gc %d from line %s,%d\n",GCCount,__FILE__,__LINE__);   \
        ensureMemory(__FILE__,__LINE__,size,item,##__VA_ARGS__);                  \
        }                                                       \
    }

/**** STACK functions ****/

/* Useful to check to see if someone is playing around with stacks */

#define SHADOW                  ShadowStack[THREAD_ID]
#define SHADOW_SPOT             ShadowSpot[THREAD_ID]

/* Generic Push and Pop Macros */
#define G_PUSH(G_STACK,G_SPOT,G_ITEM)   \
do                                      \
    {                                   \
        G_STACK[G_SPOT] = G_ITEM;       \
        ++G_SPOT;                       \
    }while(0)

#define G_POP(G_STACK,G_SPOT) G_STACK[--G_SPOT]

/* Given a stack G_STACK, a spot G_STOP, and a value G_VALUE, 
 * save the new value at the spot in the stack
 */
#define G_REPLACE(G_STACK,G_SPOT,G_VALUE) G_STACK[G_SPOT]=G_VALUE

/* Shadow Stack Macros */
#define SHADOW_PUSH(ITEM)           G_PUSH(SHADOW,SHADOW_SPOT,ITEM)
#define SHADOW_POP()                G_POP(SHADOW,SHADOW_SPOT)
#define SHADOW_REPLACE(BACK,VALUE)  G_REPLACE(SHADOW,BACK,VALUE)

/* Test to see if the two cells differ */
#define STACK_DIFFER(SPOT) \
    (                                               \
     (type(STACK[SPOT])!=SHADOW[SPOT].type)     ||  \
     (STACK_SPOT!=SHADOW_SPOT)                      \
   )

/* Replace a value on the stack with a new one, we pass in the value 
 * and how far back to go
 */
#define REPLACE(B,V)                                        \
do                                                          \
    {                                                       \
    int BACK = (B);                                         \
    int VAL  = (V);                                         \
    int ST_BACK = (STACK_SPOT-BACK-1);                      \
    G_REPLACE(STACK,ST_BACK,VAL);                           \
    /*printf("%s,%d: ",__FILE__,__LINE__); */               \
    /*printf("replacing: %d ",BACK);  */                    \
    /*ppLevel(stdout,VAL,0); */                             \
    /*printf("\n"); */                                      \
    if(StackDebugging)                                      \
        {                                                   \
        int SH_BACK = (SHADOW_SPOT-BACK-1);                 \
        if(ST_BACK!=SH_BACK)                                \
            {                                               \
            Fatal("StackSpot & ShadowSpot : %d,%d\n",       \
                    STACK_SPOT,SHADOW_SPOT);                \
            }                                               \
        S_CELL C;                                           \
                                                            \
        C.fileChanged = __FILE__;                           \
        C.lineChanged = __LINE__;                           \
                                                            \
        C.addr = VAL;                                       \
        C.type = type(VAL);                                 \
                                                            \
        C.line  = line(VAL);                                \
        C.file  =  file(VAL);                               \
        C.count = count(VAL);                               \
                                                            \
        C.ival  = ival(VAL);                                \
        C.rval  = rval(VAL);                                \
        C.fval  = fval(VAL);                                \
                                                            \
        C.cdr  = cdr(VAL);                                  \
                                                            \
        G_REPLACE(SHADOW,SH_BACK,C);                        \
        if(STACK_DIFFER(ST_BACK))                           \
            {                                               \
            P_P();                                          \
            printf("REP(%d<-%d):"#B":"#V":[%s,%d]\n",       \
                ST_BACK,                                    \
                VAL,                                        \
                __FILE__,                                   \
                __LINE__);                                  \
            P_V();                                          \
            Fatal("REPLACE(%d,%d)",STACK_SPOT,SHADOW_SPOT); \
            }                                               \
        }                                                   \
    } while(0)

#define UPDATE(B)                                           \
do                                                          \
    {                                                       \
    int BACK = (B);                                         \
    int ST_BACK = (STACK_SPOT-BACK-1);                      \
    printf("%s,%d: ",__FILE__,__LINE__);\
    printf("updating: %d ",BACK);\
    ppLevel(stdout,STACK[ST_BACK],0);\
    printf("\n");\
    } while(0)

/* Peek into the stack a depth of index */
#define PEEK(index) (STACK[STACK_SPOT-(index)-1])

/* 
 * Remove and return top element on the stack
 * TODO : Underflow detection.  See if other threads pop off stack (impossible)
 */

#define POP() G_POP(STACK,STACK_SPOT);                  \
do                                                      \
    {                                                   \
    /* STACKCHECK */                                    \
    /* printf("%s,%d: ",__FILE__,__LINE__ - 1); */      \
    /* debug("popping",STACK[STACK_SPOT]); */           \
    if(StackDebugging)                                  \
        {                                               \
        int ST = STACK[STACK_SPOT];                     \
        int SH = G_POP(SHADOW,SHADOW_SPOT).addr;        \
        char *st_t = type(ST);                          \
        char *sh_t = type(SH);                          \
                                                        \
        if(ST != SH || st_t != sh_t)                    \
            {                                           \
            P_P();                                      \
                                                        \
            printf("POP : Thread %d [%s,%d]\n",         \
                    THREAD_ID,                          \
                    __FILE__,                           \
                    __LINE__);                          \
                                                        \
            printf("\tStack :\n");                      \
            printf("\t\tAddress = %d\n",ST);            \
            printf("\t\tType = %s\n",type(ST));         \
            printf("\t\tFile = %d\n",file(ST));         \
            printf("\t\tLine = %d\n",line(ST));         \
            printf("\t\tCount = %d\n",count(ST));       \
            printf("\t\tIval = %d\n",ival(ST));         \
            printf("\t\tRval = %0.5f\n",rval(ST));      \
            printf("\t\tFval = %p\n",fval(ST));         \
            printf("\t\tCdr = %d\n\n",cdr(ST));         \
                                                        \
            S_CELL C = SHADOW[SHADOW_SPOT];             \
            printf("\tShadow :\n");                     \
            printf("\t\tAddress = %d\n",C.addr);        \
            printf("\t\tType = %s\n",C.type);           \
            printf("\t\tFile = %d\n",C.file);           \
            printf("\t\tLine = %d\n",C.line);           \
            printf("\t\tCount = %d\n",C.count);         \
            printf("\t\tIval = %d\n",C.ival);           \
            printf("\t\tRval = %0.5f\n",C.rval);        \
            printf("\t\tFval = %p\n",C.fval);           \
            printf("\t\tCdr = %d\n\n",C.cdr);           \
                                                        \
            printf("\t\tFile = %s\n",C.fileChanged);    \
            printf("\t\tLine = %d\n\n",C.lineChanged);  \
                                                        \
            P_V();                                      \
            Fatal("Spots: Stack=%d, Shadow=%d\n",       \
                    STACK_SPOT,SHADOW_SPOT);            \
            }                                           \
        }                                               \
    }while(0)


/* Push an item onto the stack, also if Debugging output if we are pushing 
 * someone elses data onto our stack.
 */

#define PUSH(LOC)                                                           \
    do                                                                      \
    {                                                                       \
    if(STACK_SPOT==StackSize)                                               \
        {                                                                   \
        Fatal("Stack overflow. Infinite recursion?\n");                     \
        }                                                                   \
    int VAL = (LOC);                                                        \
    G_PUSH(STACK,STACK_SPOT,VAL);                                           \
    /* STACKCHECK */                                                        \
    /* printf("%s,%d: ",__FILE__,__LINE__); */                              \
    /* debug("pushing",STACK[(STACK_SPOT)-1]); */                           \
    if(StackDebugging)                                                      \
        {                                                                   \
                                                                            \
        S_CELL C;                                                           \
                                                                            \
        C.fileChanged = __FILE__;                                           \
        C.lineChanged = __LINE__;                                           \
                                                                            \
        C.addr = VAL;                                                       \
        C.type = type(VAL);                                                 \
                                                                            \
        C.line  = line(VAL);                                                \
        C.file  =  file(VAL);                                               \
        C.count = count(VAL);                                               \
                                                                            \
        C.ival  = ival(VAL);                                                \
        C.rval  = rval(VAL);                                                \
        C.fval  = fval(VAL);                                                \
                                                                            \
        C.cdr  = cdr(VAL);                                                  \
        G_PUSH(SHADOW,SHADOW_SPOT,C);                                       \
                                                                            \
        if(STACK_DIFFER(STACK_SPOT-1))                                      \
            {                                                               \
            P_P();                                                          \
            printf("PUS(%d<-%d):"#LOC":[%s,%d]\n",                          \
                STACK_SPOT-1,                                               \
                LOC,                                                        \
                __FILE__,                                                   \
                __LINE__);                                                  \
            P_V();                                                          \
            Fatal("PUSH : Shadow and live are different\n");                \
            }                                                               \
        }                                                                   \
    } while(0)

/* TODO : Clean this up, decide on how to implement in best way */

#define POPN(count)                                                         \
do                                                                          \
    {                                                                       \
    int CALC = count;                                                       \
    /* STACKCHECK */                                                        \
    /* int i; */                                                            \
    /* for (i = 0; i < CALC; ++i) */                                        \
    /*     { */                                                             \
    /*     printf("%s,%d: ",__FILE__,__LINE__); */                          \
    /*     debug("popping",STACK[(STACK_SPOT)-1-i]); */                     \
    /*     } */                                                             \
    STACK_SPOT-=CALC;                                                       \
    SHADOW_SPOT-=CALC;                                                      \
    }while (0)


/* common symbols used in checking various situations */

extern int VarSymbol;
extern int ContextSymbol;
extern int CodeSymbol;
extern int ThisSymbol;
extern int ParametersSymbol;
extern int ThunkSymbol;
extern int NameSymbol;
extern int EnvSymbol;
extern int ClosureSymbol;
extern int BuiltInSymbol;
extern int ConstructorSymbol;
extern int ObjectSymbol;
extern int TypeSymbol;
extern int LabelSymbol;
extern int ValueSymbol;
extern int TraceSymbol;
extern int ThrowSymbol;
extern int ErrorSymbol;
extern int QuoteSymbol;
extern int ParenSymbol;
extern int AnonymousSymbol;
extern int LambdaSymbol;
extern int DollarSymbol;
extern int AtSymbol;
extern int SharpSymbol;
extern int UninitializedSymbol;
extern int ErrorSymbol;
extern int BeginSymbol;
extern int ScopeSymbol;
extern int BackquoteSymbol;
extern int CommaSymbol;
extern int InputPortSymbol;
extern int OutputPortSymbol;
extern int EofSymbol;
extern int ElseSymbol;
extern int NilSymbol;
extern int TrueSymbol;
extern int FalseSymbol;
extern int DefineSymbol;
extern int ExceptionSymbol;
extern int ParallelExceptionSymbol;
extern int MathExceptionSymbol;
extern int LexicalExceptionSymbol;
extern int SyntaxExceptionSymbol;
extern int EmptyExpressionSymbol;
extern int NonFunctionSymbol;
extern int NonObjectSymbol;
extern int ReturnSymbol;
extern int LevelSymbol;
extern int EqSymbol;
extern int EqEqSymbol;
extern int DotSymbol;
extern int AssignSymbol;
extern int UndefinedVariableSymbol;
extern int UninitializedVariableSymbol;
extern int XcallSymbol;
extern int PrettyStatementSymbol;

extern int readIndex;
extern int writeIndex;
extern int appendIndex;
extern int stdinIndex;
extern int stdoutIndex;
extern int stderrIndex;

extern int nilIndex;
extern int trueIndex;
extern int falseIndex;

extern int integerZero;
extern int integerOne;

extern int SetBangSymbol;
extern int AndAndSymbol;
extern int OrOrSymbol;
extern int GtSymbol;
extern int GteSymbol;
extern int LtSymbol;
extern int LteSymbol;
extern int NeqSymbol;
extern int FunctionSymbol;
extern int HeadAssignSymbol;
extern int TailAssignSymbol;
extern int OpenBracketSymbol;

#endif
