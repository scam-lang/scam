/*
 *  Main Author : John C. Lusth
 *  Barely Authors : Jeffrey Robinson, Gabriel Loewen
 *  Last Edit : May 4, 2014
 *
 *  Memory management functions.
 *      Request memory
 *      Garbage collection
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/time.h>

#include <unistd.h>

#include "scam.h"
#include "cell.h"
#include "types.h"
#include "util.h"
#include "env.h"
#include "pp-base.h"
#include "stack.h"
#include "thread.h"

#define MOVED 1
#define UNMOVED 0

#define MAX_THREAD_TIMEOUT 10.0

/* number of garbage collections */
int GCCount = 0;

/* Debugging functions for the stack */
void saveMTStack(char *, int, int);
void saveStack(char*,int);
void diff(char*,char*,int);

/* GC */
static void MarkCompact(int,int);
static void StopAndCopy();

void markObject(int,int);
void markRoots(int);
void scanFree(void);
void computeLocations();
void updateReferences();
void relocate(int,int);
int moveStackItem(int);

/* Multi-threading GC */
int memAvailable(int);
void threadSafeEnsure(char *,int,int);
void threadSafeContiguousEnsure(char *,int,int);

/* FreeList  */
int FreeList;
void freePush(int);
int freePop();
int FreeCount;

/* global constants */
char **SymbolTable;
int SymbolCount;        /* How many symbols are in the sysmte */
int HeapBottom;         /* Symbols inserted from scamInit */
int GCQueueCount;       /* How many people are currently waiting on a GC*/

/* Symbols parameters */
static int SymbolSize = 100;
static int SymbolIncrement = 100;

/* semispace for stop and copy */
CELL theCars;
CELL newCars;

int MemorySpot;
static int newMemorySpot;

#define  MEMORY_SIZE (2000000)
int MemorySize = MEMORY_SIZE;

/* Stack */
int StackSize = 1024 * 32;
int *Stack[MAX_THREADS];
int StackSpot[MAX_THREADS];

/* Shadow stacking */
S_CELL *ShadowStack[MAX_THREADS];
int ShadowSpot[MAX_THREADS];

/* Symbols */
int AndAndSymbol;
int AnonymousSymbol;
int AssignSymbol;
int AtSymbol;
int BackquoteSymbol;
int BeginSymbol;
int BuiltInSymbol;
int ClosureSymbol;
int CodeSymbol;
int CommaSymbol;
int ConstructorSymbol;
int ContextSymbol;
int DefineSymbol;
int DollarSymbol;
int DotSymbol;
int ElseSymbol;
int EnvSymbol;
int EofSymbol;
int EqEqSymbol;
int EqSymbol;
int ErrorSymbol;
int ErrorSymbol;
int ExceptionSymbol;
int FalseSymbol;
int FunctionSymbol;
int GtSymbol;
int GteSymbol;
int HeadAssignSymbol;
int InputPortSymbol;
int LabelSymbol;
int LambdaSymbol;
int LevelSymbol;
int LexicalExceptionSymbol;
int LtSymbol;
int LteSymbol;
int MathExceptionSymbol;
int NameSymbol;
int NeqSymbol;
int NilSymbol;
int NonFunctionSymbol;
int NonObjectSymbol;
int ObjectSymbol;
int OpenBracketSymbol;
int OrOrSymbol;
int OutputPortSymbol;
int ParallelExceptionSymbol;
int ParametersSymbol;
int ParenSymbol;
int PrettyStatementSymbol;
int QuoteSymbol;
int ReturnSymbol;
int ScopeSymbol;
int SetBangSymbol;
int SharpSymbol;
int SyntaxExceptionSymbol;
int EmptyExpressionSymbol;
int TailAssignSymbol;
int ThisSymbol;
int ThrowSymbol;
int ThunkSymbol;
int TraceSymbol;
int TrueSymbol;
int TypeSymbol;
int UndefinedVariableSymbol;
int UninitializedSymbol;
int UninitializedVariableSymbol;
int ValueSymbol;
int VarSymbol;
int XcallSymbol;

/*
 * TODO : Debate if this is good programming 
 * Symbols have an index, used when convenient
 */

int readIndex;
int writeIndex;
int appendIndex;
int stdinIndex;
int stdoutIndex;
int stderrIndex;

int nilIndex;
int trueIndex;
int falseIndex;

int integerZero;
int integerOne;

static double getTime(void);

/* scamInit - Initialize memory and setup the scam running environment */

void
scamInit(int memSize)
    {

    /* Take in argument if someone passed it in */
    if(memSize > 0 )
        {
        MemorySize = memSize;
        }

    /* Split memory in two if using STOP_AND_COPY */
    if (GCMode != STOP_AND_COPY)
        {
        MemorySize *= 2;
        }

    /* Initialize the global Symbol table */
    SymbolTable = (char **) malloc(SymbolSize * sizeof(char *));
    if (SymbolTable == 0)
        {
        Fatal("could not allocate Symbol table\n");
        }
    memset(SymbolTable, -1, sizeof(char*)*SymbolSize);
    
    threadingInit();

    /* Initialize memory for main thread */

    memoryInit();

    /* Add Symbols to SymbolTable */

    /* TODO : Why does it have to be first? */
    /* nil has to be the first symbol (Because you check for 0 isn't it) */
    NilSymbol            = newSymbol("nil");

    ASSERT(MEMORY_SPOT == 1);

    FalseSymbol          = newSymbol("#f");
    TrueSymbol           = newSymbol("#t");

    AnonymousSymbol      = newSymbol("anonymous");
    AssignSymbol         = newSymbol("assign");
    AtSymbol             = newSymbol("@");
    BackquoteSymbol      = newSymbol("backquote");
    BeginSymbol          = newSymbol("begin");
    BuiltInSymbol        = newSymbol("builtIn");
    ClosureSymbol        = newSymbol("closure");
    CodeSymbol           = newSymbol("code");
    CommaSymbol          = newSymbol("comma");
    ConstructorSymbol    = newSymbol("__constructor");
    ContextSymbol        = newSymbol("__context");
    DefineSymbol         = newSymbol("define");
    DollarSymbol         = newSymbol("$");
    DotSymbol            = newSymbol(".");
    ElseSymbol           = newSymbol("else");
    EnvSymbol            = newSymbol("environment");
    EofSymbol            = newSymbol("EOF");
    EqEqSymbol           = newSymbol("==");
    EqSymbol             = newSymbol("=");
    ErrorSymbol          = newSymbol("error");
    ExceptionSymbol      = newSymbol("generalException");
    InputPortSymbol      = newSymbol("inputPort");
    LabelSymbol          = newSymbol("__label");
    LambdaSymbol         = newSymbol("lambda");
    LevelSymbol          = newSymbol("__level");
    LexicalExceptionSymbol = newSymbol("lexicalException");
    MathExceptionSymbol  = newSymbol("mathException");
    NameSymbol           = newSymbol("name");
    NonFunctionSymbol    = newSymbol("nonFunction");
    NonObjectSymbol      = newSymbol("nonObject");
    ObjectSymbol         = newSymbol("__object");
    OutputPortSymbol     = newSymbol("outputPort");
    ParallelExceptionSymbol      = newSymbol("parallelException");
    ParametersSymbol     = newSymbol("parameters");
    ParenSymbol          = newSymbol("paren");
    QuoteSymbol          = newSymbol("quote");
    ReturnSymbol         = newSymbol("return");
    ScopeSymbol          = newSymbol("scope");
    SharpSymbol          = newSymbol("#");
    SyntaxExceptionSymbol = newSymbol("syntaxException");
    EmptyExpressionSymbol = newSymbol("emptyExpression");
    ThisSymbol           = newSymbol("this");
    ThrowSymbol          = newSymbol("throw");
    ThunkSymbol          = newSymbol("thunk");
    TraceSymbol          = newSymbol("trace");
    TypeSymbol           = newSymbol("type");
    UndefinedVariableSymbol = newSymbol("undefinedVariable");
    UninitializedSymbol  = newSymbol(":UNINITIALIZED:");
    UninitializedVariableSymbol = newSymbol("uninitializedVariable");
    ValueSymbol          = newSymbol("value");
    VarSymbol            = newSymbol("var");

    AndAndSymbol         = newSymbol("&&");
    FunctionSymbol       = newSymbol("function");
    GtSymbol             = newSymbol(">");
    GteSymbol            = newSymbol(">=");
    HeadAssignSymbol     = newSymbol("head=");
    LtSymbol             = newSymbol("<");
    LteSymbol            = newSymbol("<=");
    NeqSymbol            = newSymbol("!=");
    OpenBracketSymbol    = newSymbol("[");
    OrOrSymbol           = newSymbol("||");
    PrettyStatementSymbol= newSymbol("prettyStatement");
    SetBangSymbol        = newSymbol("set!");
    TailAssignSymbol     = newSymbol("tail=");
    XcallSymbol          = newSymbol("__xcall");

    nilIndex             = ival(NilSymbol);
    trueIndex            = ival(TrueSymbol);
    falseIndex           = ival(FalseSymbol);
    readIndex            = findSymbol("read");
    writeIndex           = findSymbol("write");
    appendIndex          = findSymbol("append");
    stdinIndex           = findSymbol("stdin");
    stdoutIndex          = findSymbol("stdout");
    stderrIndex          = findSymbol("stderr");

    integerZero          = newInteger(0);
    integerOne           = newInteger(1);

    /* Save where we are so we don't have to keep copying over the Symbols */
    NEW_MEM_SPOT = HeapBottom = MEMORY_SPOT;

    /* If Stop and Copy, copy over Symbol table to "shadow" cars */
    int i;
    if (GCMode == STOP_AND_COPY)
        {
        for(i = 0; i < HeapBottom; i++ )
            {
            newtype(i) = type(i);

            newline(i) = line(i);
            newfile(i) = file(i);
            newcount(i) = count(i);

            newival(i) = ival(i);
            newrval(i) = rval(i);
            newfval(i) = fval(i);

            newstatus(i) = status(i);

            newcdr(i) = cdr(i);
            }
        }
    }

/*
 *  scamShutdown - free all memory and release all resources.
 *      Remember that this has to be in reverse order! 
 */

void
scamShutdown()
    {
    memoryShutdown();
    threadingShutdown();
    free(SymbolTable);
    }

/*
 * memoryInit - initialize memory for current thread, special case when main thread.
 *
 */

void
memoryInit()
    {

    /* Only create the stack if main thread, this might
        be changed later to have each thread have it's 
        own stack.  
     */

    if (THREAD_ID == 0)
        {
        /* Allocate heap space */
        THE_CARS.type        = malloc(sizeof(char*) * MemorySize);

        THE_CARS.line        = malloc(sizeof(short) * MemorySize);
        THE_CARS.file        = malloc(sizeof(short) * MemorySize);
        THE_CARS.count       = malloc(sizeof(int) * MemorySize);

        THE_CARS.ival        = malloc(sizeof(int) * MemorySize);
        THE_CARS.rval        = malloc(sizeof(double) * MemorySize);
        THE_CARS.fval        = malloc(sizeof(int (**)(int,int)) * MemorySize);

        THE_CARS.status      = malloc(sizeof(char) * MemorySize);

        /* Used to output who created a cons cell */
        if(StackDebugging)
            {
            THE_CARS.creator    = malloc(sizeof(int) * MemorySize);
            THE_CARS.lastEditor = malloc(sizeof(int) * MemorySize);
            THE_CARS.lastFile   = malloc(sizeof(char*)* MemorySize);
            THE_CARS.lastLine   = malloc(sizeof(int)  * MemorySize);
            }

        /* Stop and copy requires a forwarding address */
        if(GCMode != STOP_AND_COPY)
            {
            THE_CARS.fwd     = malloc(sizeof(int) * MemorySize);
            }

        THE_CARS.cdr         = malloc(sizeof(int) * MemorySize);

        /* Only need to test THE_CARS.cdr */
        if (THE_CARS.cdr  == NULL)
            {
            Fatal("could not allocate code segment (cars and cdrs)\n");
            }

        /* Make sure everything is 0 */
        memset(THE_CARS.type , 0 , sizeof(char*) * MemorySize);

        memset(THE_CARS.line , 0 , sizeof(short) * MemorySize);
        memset(THE_CARS.file , 0 , sizeof(short) * MemorySize);
        memset(THE_CARS.count , -1 , sizeof(int) * MemorySize);

        memset(THE_CARS.ival , -1 , sizeof(int) * MemorySize);
        memset(THE_CARS.rval , 0 , sizeof(double) * MemorySize);
        memset(THE_CARS.fval , -1 , sizeof(int (**)(int,int)) * MemorySize);

        /* Default to unmoved, otherwise we will not move it when we GC  */
        memset(THE_CARS.status , UNMOVED , sizeof(char) * MemorySize);
        if(GCMode != STOP_AND_COPY)
            {
            memset(THE_CARS.fwd , -1 , sizeof(int) * MemorySize);
            }
        if(StackDebugging)
            {
                memset(THE_CARS.creator , -1 , sizeof(int) * MemorySize);
                memset(THE_CARS.lastEditor , -1 , sizeof(int) * MemorySize);
                memset(THE_CARS.lastFile , -1 , sizeof(char) * MemorySize);
                memset(THE_CARS.lastLine , -1 , sizeof(int) * MemorySize);
            }

        memset(THE_CARS.cdr , -1 , sizeof(int) * MemorySize);

        /* Allocate memory for semispace */
        if (GCMode == STOP_AND_COPY)
            {
            NEW_CARS.type       = malloc(sizeof(char*) * MemorySize);

            NEW_CARS.line       = malloc(sizeof(short) * MemorySize);
            NEW_CARS.file       = malloc(sizeof(short) * MemorySize);
            NEW_CARS.count      = malloc(sizeof(int) * MemorySize);

            NEW_CARS.ival       = malloc(sizeof(int) * MemorySize);
            NEW_CARS.rval       = malloc(sizeof(double) * MemorySize);
            NEW_CARS.fval       = malloc(sizeof(int (**)(int,int)) * MemorySize);

            NEW_CARS.status     = malloc(sizeof(char) * MemorySize);
            if (StackDebugging)
                {
                NEW_CARS.creator    = malloc(sizeof(int) * MemorySize);
                NEW_CARS.lastEditor = malloc(sizeof(int) * MemorySize);
                NEW_CARS.lastFile   = malloc(sizeof(char*)* MemorySize);
                NEW_CARS.lastLine   = malloc(sizeof(int)  * MemorySize);
                }
            NEW_CARS.fwd        = malloc(sizeof(int) * MemorySize);
            
            NEW_CARS.cdr        = malloc(sizeof(int) * MemorySize);

            if (NEW_CARS.cdr == NULL)
                {
                Fatal("Could not allocate memory for semispace.\n");
                }

            memset(NEW_CARS.type ,  0 , sizeof(char*) * MemorySize);

            memset(NEW_CARS.line , -1 , sizeof(short) * MemorySize);
            memset(NEW_CARS.file , -1 , sizeof(short) * MemorySize);
            memset(NEW_CARS.count , -1 , sizeof(int) * MemorySize);

            memset(NEW_CARS.ival , -1 , sizeof(int) * MemorySize);
            memset(NEW_CARS.rval , 0 , sizeof(double) * MemorySize);
            memset(NEW_CARS.fval , -1 , sizeof(int (**)(int,int)) * MemorySize);

            /* If not set then GC will not move anything  */
            memset(NEW_CARS.status , UNMOVED , sizeof(char) * MemorySize);
            memset(NEW_CARS.fwd , -1 , sizeof(int) * MemorySize);
            if(StackDebugging)
                {
                memset(NEW_CARS.creator , -1 , sizeof(int) * MemorySize);
                memset(NEW_CARS.lastEditor , -1 , sizeof(int) * MemorySize);
                memset(NEW_CARS.lastFile , -1 , sizeof(char) * MemorySize);
                memset(NEW_CARS.lastLine , -1 , sizeof(int) * MemorySize);
                }

            memset(NEW_CARS.cdr , -1 , sizeof(int) * MemorySize);
            }

        /* Initialize the free list to be empty */
        FREE_LIST = 0;
        }

    /* Setup the stack for this thread */
    STACK = (int *) malloc(sizeof(int) * StackSize);
    if (STACK == NULL)
        {
        Fatal("could not allocate stack\n");
        }
    memset(STACK ,-1 ,sizeof(int) * StackSize);

    /* Initial spot on Stack */
    STACK_SPOT = 0;

    if (StackDebugging)
        {
        /* The shadow stack is used to make sure no one is playing with the stack */
        SHADOW = malloc(sizeof(S_CELL)*StackSize);
        if(SHADOW==NULL)
        {
            Fatal("Could not allocate shadow stack\n");
        }
        memset(SHADOW , 0 ,sizeof(S_CELL) * StackSize);

        SHADOW_SPOT = 0;
        }
    }


/*
 *  memoryShutdown - Releases all thread specific memory, if main thread 
 *      we also release globally allocated memory.
 */

void
memoryShutdown()
    {
    /* Thread 0 is the main thread, it means the program is exited */
    if(THREAD_ID == 0)
        {
        free(THE_CARS.type);

        free(THE_CARS.file);
        free(THE_CARS.line);
        free(THE_CARS.count);

        free(THE_CARS.ival);
        free(THE_CARS.rval);
        free(THE_CARS.fval);

        free(THE_CARS.status);
        if(StackDebugging)
            {
            free(THE_CARS.creator);
            free(THE_CARS.lastEditor);
            free(THE_CARS.lastFile);
            free(THE_CARS.lastLine);
            }
            
        if(GCMode!=STOP_AND_COPY)
            {
            free(THE_CARS.fwd);
            }

        free(THE_CARS.cdr);

        if (GCMode == STOP_AND_COPY)
            {
            free(NEW_CARS.type);

            free(NEW_CARS.line);
            free(NEW_CARS.file);
            free(NEW_CARS.count);

            free(NEW_CARS.ival);
            free(NEW_CARS.rval);
            free(NEW_CARS.fval);

            free(NEW_CARS.status);

            if(StackDebugging)
                {
                free(NEW_CARS.creator);
                free(NEW_CARS.lastEditor);
                free(NEW_CARS.lastFile);
                free(NEW_CARS.lastLine);
                }

            free(NEW_CARS.fwd);
            free(NEW_CARS.cdr);
            }
        }

    free(STACK);
    STACK = NULL;
    if (StackDebugging)
        {
        STACK_SPOT = -1;
        free(SHADOW);
        SHADOW = NULL;
        SHADOW_SPOT = -1;
        }
    }

/* Generic cons "function" */

#define _cons(CAR,CDR,TYPE,F,L,RES,FL,LN)                           \
    {                                                               \
    int p = 0;                                                      \
                                                                    \
    /* caller is responsible for ensuring memory available */       \
    if (GCMode == MARK_SWEEP )                                      \
        {                                                           \
        if (FREE_LIST != 0)                                         \
            {                                                       \
            RES = freePop();                                        \
            p = 1;                                                  \
            }                                                       \
        else                                                        \
            {                                                       \
            RES = MEMORY_SPOT;                                      \
            ++MEMORY_SPOT;                                          \
            }                                                       \
        }                                                           \
    else if (GCMode == STOP_AND_COPY)                               \
        {                                                           \
        RES = MEMORY_SPOT;                                          \
        setstatus(RES, UNMOVED);                                    \
        ++MEMORY_SPOT;                                              \
        }                                                           \
                                                                    \
    if(RES == MemorySize)                                           \
        {                                                           \
        ASSERT(RES < MemorySize);                                   \
        }                                                           \
                                                                    \
    if(StackDebugging)                                              \
        {                                                           \
        if (creator(RES) != -1 && p==0)                             \
            {                                                       \
            Fatal("Tried to reallocate cell %d.\n"                  \
                "    original allocation: thread %d,%s,%d\n"        \
                "    new allocation:      thread %d,%s,%d\n"        \
                "    gc count:            %d\n",                    \
                RES,                                                \
                creator(RES),lastFile(RES),lastLine(RES),           \
                THREAD_ID,__FILE__,__LINE__,GCCount);               \
            }                                                       \
        creator(RES)    = THREAD_ID;                                \
        lastEditor(RES) = THREAD_ID;                                \
        lastFile(RES)   = FL;                                       \
        lastLine(RES)   = LN;                                       \
        }                                                           \
                                                                    \
    settype(RES,TYPE);                                              \
    setfile(RES,F);                                                 \
    setline(RES,L);                                                 \
    setcar(RES,CAR);                                                \
    setcdr(RES,CDR);                                                \
    }

/*
 * cons - the main memory allocation routine
 *      - the source file location of the new cell is tagged with the car
 *        item's location
 *
 *  Memory Guarantees - Calling function
 *
 */

int
t_cons(int a,int b,char* f, int l)
    {
    int ret = 0;
    if(P_REQ[THREAD_ID] != ACQUIRED)
        {
        P_P();
        Fatal("t_cons: thread %d does not hold the lock (%s,%d)\n",
            THREAD_ID,f,l);
        P_V();
        }
    _cons(a, b, CONS, file(a), line(a), ret,__FILE__,__LINE__); //this macro sets ret
    return ret;
    }

/*
 * cons2 - like cons, but the cdr item transfers its location (file,line)
 *         in the source file
 *
 *  Memory Guarantees - Calling function
 *
 */

int
cons2(int a,int b)
    {
    int ret = 0;
    _cons(a,b,CONS,file(b),line(b),ret,__FILE__,__LINE__); //this macro sets ret
    return ret;
    }

/*
 * consfl - like cons, but source file location (file,line) is passed in
 *
 *  Memory Guarantees - Calling function
 *
 */

int
consfl(int a,int b,int fileIndex,int lineNumber)
    {
    int ret = 0;
    _cons(a,b,CONS,fileIndex,lineNumber,ret,__FILE__,__LINE__); //this macro sets ret
    return ret;
    }

/*
 * append - join two lists together
 *        - append is destructive! -- the first list is modified
 *
 *  Memory Guarantees - Calling function
 *
 */

int
append(int list1,int list2)
    {
    int start = list1;
    while (cdr(list1) != 0)
        {
        list1 = cdr(list1);
        }

    setcdr(list1,list2);

    return start;
    }

/*
 * newString - allocates a Scam string from the given C string
 *
 *  Memory Guarantees - This function handles memory allocation
 *
 */

int
newString(char *s)
    {
    int start,length;
    
    ASSERT( s != 0);

    length = strlen(s);

    ASSERT(length > 0);

    P();
    ENSURE_CONTIGUOUS_MEMORY(length+1, (int *)0);

    start = MEMORY_SPOT;

    while (*s != 0)
        {
        if (StackDebugging)
            {
            creator(MEMORY_SPOT) = THREAD_ID;
            setEditor(MEMORY_SPOT,THREAD_ID);
            setLastFile(MEMORY_SPOT,__FILE__);
            setLastLine(MEMORY_SPOT,__LINE__);
            }

        settype(MEMORY_SPOT,STRING);
        setival(MEMORY_SPOT,*s);
        setcount(MEMORY_SPOT,length);

        setline(MEMORY_SPOT,0);
        setfile(MEMORY_SPOT,0);

        setcdr(MEMORY_SPOT,MEMORY_SPOT+1);

        --length;
        ++s;
        ++MEMORY_SPOT;
        }
    setcdr(MEMORY_SPOT - 1,0);
    V();

    return start;
    }

/* newStringUnsafe
 *
 * gc unsafe version of newString, used when initial environments are populated
 *
 */

int
newStringUnsafe(char *s)
    {
    int start,length;

    ASSERT(s != 0); 

    length = strlen(s);
    ASSERT(length > 0); 

    start = MEMORY_SPOT;

    while (*s != 0)
        {
        if (StackDebugging)
            {
            creator(MEMORY_SPOT) = THREAD_ID;
            setEditor(MEMORY_SPOT,THREAD_ID);
            setLastFile(MEMORY_SPOT,__FILE__);
            setLastLine(MEMORY_SPOT,__LINE__);
            }

        settype(MEMORY_SPOT,STRING);
        setival(MEMORY_SPOT,*s);
        setcount(MEMORY_SPOT,length);

        setline(MEMORY_SPOT,0);
        setfile(MEMORY_SPOT,0);

        setcdr(MEMORY_SPOT,MEMORY_SPOT+1);

        --length;
        ++s;
        ++MEMORY_SPOT;
        }
    setcdr(MEMORY_SPOT - 1,0);

    return start;
    }

/*
 * newSymbol - allocates a Scam symbol from the given C string
 *           - Scam symbols are interned, so the symbol is an index into a
 *             repository of strings
 *
 *  Memory Guarantees - Allocated here
 *
 */

int
newSymbol(char *s)
    {
    int index = findSymbol(s);
    int ret = 0;
    P();
    ENSURE_MEMORY(1,(int *) 0);
    _cons(index,0,SYMBOL,0,0,ret,__FILE__,__LINE__); //this macro sets ret
    V();
    return ret;
    }

/* newSymbolUnsafe
 *
 * gc unsafe version of newSymbol, used when initial environments are populated
 *
 */

int
newSymbolUnsafe(char *s)
    {
    int index = findSymbol(s);
    int ret = 0;
    _cons(index,0,SYMBOL,0,0,ret,__FILE__,__LINE__); //this macro sets ret
    return ret;
    }

/*
 * newInteger - allocates a Scam integer from the given C integer
 *
 *  Memory Guarantees - Allocated here
 *
 */

int
newInteger(int i)
    {
    int ret = 0;
    P();
    ENSURE_MEMORY(1,(int *) 0);
    _cons(i,0,INTEGER,0,0,ret,__FILE__,__LINE__); //this macro sets ret
    V();
    return ret;
    }

int
newIntegerUnsafe(int i)
    {
    int ret = 0;
    _cons(i,0,INTEGER,0,0,ret,__FILE__,__LINE__); //this macro sets ret
    return ret;
    }

/*
 * newReal - allocates a Scam real from the given C real
 *
 *  Memory Guarantees - Allocated here
 *
 */

int
newReal(double r)
    {
    int ret = 0;
    P();
    ENSURE_MEMORY(1,(int *) 0);
    _cons(0,0,REAL,0,0,ret,__FILE__,__LINE__); //this macro sets ret
    V();
    setrval(ret,r);
    return ret;
    }

/*
 * newPunctuation - allocates a lexeme with the given type
 *                - used by the lexer for punctuation like '('
 *
 *  Memory Guarantees - Allocated here
 *
 */

int
newPunctuation(char *t)
    {
    int ret = 0;
    P();
    ENSURE_MEMORY(1,(int *) 0);
    _cons(0,0,t,0,0,ret,__FILE__,__LINE__); //this macro sets ret
    V();
    return ret;
    }

/*
 * cellString - translates a Scam string into a C character array
 *            - the array can be passed in or a static internal
 *              array can be used
 *            - this routine is not thiread safe
 *
 *  Memory Guarantees - N/A
 *
 */


char *
cellString(char *buffer,int size, int s)
    {
    int i,length;
    char *target;

    target = buffer;

    i = 0;
    length = count(s);
    while (i < size - 1 && length > 0)
        {
        target[i] = (char) ival(s);
        ++s;
        ++i;
        --length;
        }

    target[i] = '\0';

    return target;
    }

/*
 * cellStringTr - like cellString, but escaped characters are translated to
 *              - their single character equivalents, such as \n to newline
 *
 *  Memory Guarantees - N/A
 *
 */


char *
cellStringTr(char *buffer,int size, int s)
    {
    int i,length;
    char *target;
    static char store[4096];

    if (buffer == 0)
        {
        target = store;
        size = sizeof(store);
        }
    else
        {
        target = buffer;
        }

    i = 0;
    length = count(s);
    while (i < size - 1 && length > 0)
        {
        if (ival(s) == '\\')
            {
            ++s;
            if (ival(s) == 'n')
                {
                target[i] = '\n';
                }
            else if (ival(s) == 'r')
                {
                target[i] = '\r';
                }
            else if (ival(s) == 't')
                {
                target[i] = '\t';
                }
            else
                {
                target[i] = (char) ival(s);
                }
            }
        else
            {
            target[i] = (char) ival(s);
            }
        ++s;
        ++i;
        --length;
        }

    target[i] = '\0';

    return target;
    }

/*
 * findSymbol - look up an interned sybmol
 *            - if the symbol is not found, add it to the repository
 *
 *  Memory Guarantees - N/A
 *
 */

// TODO: move symbol table into Scam memory? Use a suffix tree? (Probably not for a while JR)

int
findSymbol(char *s)
    {
    int i;
    char *dup;

    for (i = 0; i < SymbolCount; ++i)
        if (strcmp(s,SymbolTable[i]) == 0)
            {
            return i;
            }

    if (SymbolCount >= SymbolSize)
        {
        char **symT;
        int symSize;
        
        symSize = SymbolSize + SymbolIncrement;
        symT = (char **)realloc(SymbolTable,sizeof(char *)*symSize);
        
        if(symT == NULL)
            {
            return throw(ExceptionSymbol,
                    "findSymbol : SymbolTable cannot grow, input = %s",
                    s);
            }

        SymbolTable = symT;
        SymbolSize  = symSize;

        }

    dup = strdup(s);
    if (SymbolTable == 0 || dup == 0)
        {
        Fatal("Symbol Table is full\n");
        }

    SymbolTable[SymbolCount++] = dup;

    return SymbolCount - 1;
    }

/*
 * length - generic length function for scam lists, arrays, and strings
 *
 */

inline int
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

/* caller is responsible for ensuring 'size' cells available */
int
allocateContiguous(char *typ,int size)
    {
    ASSERT(size > 0);

    int init = (typ == STRING) ? 'x' : integerZero;
    int start = MEMORY_SPOT;

    /* set the types and cdrs appropriately and set the counts */
    while( size > 0)
        {
        if (StackDebugging)
            {
            creator(MEMORY_SPOT) = THREAD_ID;
            setEditor(MEMORY_SPOT,THREAD_ID);
            setLastFile(MEMORY_SPOT,__FILE__);
            setLastLine(MEMORY_SPOT,__LINE__);
            }
        settype(MEMORY_SPOT,typ);
        setcar(MEMORY_SPOT,init);
        setcdr(MEMORY_SPOT,MEMORY_SPOT + 1);
        setcount(MEMORY_SPOT,size);
        ++MEMORY_SPOT;
        --size;
        }
    setcdr(MEMORY_SPOT - 1,0);
    return start;
    }


/*
 *  ensureContiguousMemory    - Saves variables on the stack in case of garbage collection.
 *                    Checks to see if there is enough contiguous memory, if not 
 *                    does a GC.
 *
 *  Note:  Must have called P() before reaching this function!
 */
void
ensureContiguousMemory(char *fileName,int lineNumber,int needed, int *item, ...)
    {
    va_list ap;
    int *store[20]; //This is bad!
    int storePtr = 0;

    va_start(ap, item);

    while (item != 0)
        {
        ASSERT(storePtr < 20);  // Saving grace!
        PUSH(*item);
        store[storePtr++] = item;
        item = va_arg(ap,int *);
        }
    threadSafeContiguousEnsure(fileName,lineNumber,needed);

    while(storePtr--)
        {
        *(store[storePtr]) = POP();
        }
    }

/*
 *  ensureMemory    - Saves variables on the stack in case of garbage collection.
 *                    Checks to see if there is enough memory, if not does a GC.
 *
 *  Note:  Must have called P() before reaching this function!
 */

void
ensureMemory(char *fileName,int lineNumber,int needed, int *item, ...)
    {
    va_list ap;
    int *store[20]; //This is bad!
    int storePtr = 0;

    va_start(ap, item);

    while (item != 0)
        {
        ASSERT(storePtr < 20);  // Saving grace!
        PUSH(*item);
        store[storePtr++] = item;
        item = va_arg(ap,int *);
        }

    threadSafeEnsure(fileName,lineNumber,needed);

    while(storePtr--)
        {
        *(store[storePtr]) = POP();
        }
    }


/*
 *  threadSafeEnsure    - Validate that we have enough memory, if not 
 *                        we do a garbage collection.  If we have more
 *                        than one thread all threads sleep save one 
 *                        who will then do the garbage collection.
 *
 *  Note: We have a lock on P when entering and leaving.
 */

void
threadSafeEnsure(char *fileName,int lineNumber,int needed)
    {
TOP1:
    // If we have enough memory and no one is trying to GC!
    if (memAvailable(needed) && GCQueueCount == 0)
        {
        return;
        }
    GC(needed,0);
    goto TOP1;
    }

/*
 *  threadSafeContiguousEnsure    - Validate that we have enough memory, if not 
 *                                  we do a garbage collection.  If we have more
 *                                  than one thread all threads sleep save one 
 *                                  who will then do the garbage collection.
 *
 *  Note: We have a lock on P when entering and leaving.
 */

void
threadSafeContiguousEnsure(char *fileName,int lineNumber,int needed)
    {
TOP2:
    // If we have enough memory and no one is trying to GC!
    if (MEMORY_SPOT + needed <= MemorySize && GCQueueCount < 1)
        {
        return;
        }
    GC(needed,1);
    goto TOP2;
    }

/*
 *  GC  : Perform a garbage collect.
 */

void 
GC(int needed, int contiguous)
    {
    static int num = 0;
    static double total = 0.0;
    double startTime,delta;

    // If we only have a single thread or everyone else is waiting!
    if (WorkingThreads < 2 || GCQueueCount == WorkingThreads - 1)
        {

        if(Debugging)
            {
            saveStack("before", GCCount);
            }

        if (GCDisplay)
            {
            startTime = getTime();
            }

        if (GCMode == MARK_SWEEP)
            {
            MarkCompact(needed, contiguous);
            }
        else if (GCMode == STOP_AND_COPY)
            {
            StopAndCopy();
            }

        if(GCDisplay)
            {
            delta = getTime() - startTime;
            total+=delta;
            printf("gc:%d, took %fs "
                "leaving %d uncontiguous and %d contiguous cells free (total: %fs)\n",
                ++num,delta,FREE_COUNT,MemorySize- MEMORY_SPOT,total);
            }

        if(StackDebugging)
            {
                saveStack("after", GCCount);
            }

        ++GCCount;
        GCQueueCount = 0;

        /* Could not allocate the memory needed */
        if ( (contiguous || FREE_COUNT < needed) && (MEMORY_SPOT + needed > MemorySize))
            {
            FILE *fp = fopen("thread.log","a");
            fprintf(fp,"thread %d: gc failed: out of memory\n",THREAD_ID);
            fflush(fp);
            fclose(fp);
            if( THREAD_ID == 0) 
                {
                printf("Out of Memory\n");
                }
            V();
            STACK_SPOT = 0;
            exit(-1);
            }

        }
    else    // Someone is working, I go on the waiting thread.
        {
        ++GCQueueCount;
        double startTime = getTime();
        V();
        /* While GC has not happened and we have someone working who busy wait (with sleep!)  */
        while (GCQueueCount > 0)
            {
            usleep(10000);
            if (!Debugging && (getTime() - startTime > MAX_THREAD_TIMEOUT))
                Fatal("deadlock detected while garbage collecting\n");
            }
        P();
        // Either someone exited so I have to do GC or someone did a GC and I should grab some memory!
        }
   RecentGC = 1;
    }

inline int 
memAvailable(int needed)
    {
    if( (MEMORY_SPOT + needed) <= MemorySize)
        {
        return 1;
        }
    if (GCMode == MARK_SWEEP)
        {
        return (FREE_COUNT >= needed);
        }
    return 0;
    }

/*******************************************************************
 *                                                                 *
 * Begin Garbage collection routines                               *
 *                                                                 *
 *******************************************************************/

static void
MarkCompact(int needed, int contiguous)
    {

    FREE_LIST = 0;
    FREE_COUNT = 0;

    markRoots(MARKED);
    scanFree();

    if (FREE_COUNT < needed || (contiguous && (MEMORY_SIZE - MEMORY_SPOT < needed)))
        {
        FREE_LIST  = 0;
        FREE_COUNT = 0;

        /* Mark all of the reachable objects */
        markRoots(MARKED);
        computeLocations();
        updateReferences(HeapBottom,MemorySize);
        relocate(HeapBottom,MemorySize);
        }
    }


/* 
 *  freePop - Return the index to the next free spot on the Free list
 */

inline int
freePop()
    {
    if (FREE_LIST==0)
        {
        Fatal("Trying to remove from empty free list.\n");
        }
    int spot = FREE_LIST;
    FREE_LIST = cdr(FREE_LIST);
    --FREE_COUNT;
    return spot;
    }

/*
 *  freePush - Push an index from the heap onto the free list 
 */

inline void
freePush(int location)
    {
    if(StackDebugging)
        {
        creator(location) = -1;
        }
    cdr(location) = FREE_LIST;
    FREE_LIST = location;
    ++FREE_COUNT;
    }

inline void
computeLocations()
    {
    register int scan = HeapBottom;
    register int free = HeapBottom;

    /* All values before scan will have an appropriate forwarding address */
    while (scan < MEMORY_SPOT)
        {
        /* Only move marked cells */
        if (ismarked(scan))
            {
            /* Free is where we will move the the object indexed by scan */
            setfwd(scan,free);
            ++free;
            }
        ++scan;
        }
    NEW_MEM_SPOT = free;
    }

/*
 *  updateReferences    -   Updates all the references in the heap
 *
 */

void
updateReferences(int start,int end)
    {
    int i,j;

    /* Go through the Root list and update references */
    for (i=0; i<CreatedThreads; ++i)
        {
        /* If this thread is active */
        for (j=0; j< StackSpot[i]; ++j)
            {
            int tmp = Stack[i][j];
            if (tmp >= HeapBottom)
                {
                tmp = fwd(tmp);
                Stack[i][j] = tmp;
                }
            if(StackDebugging)
                {
                S_CELL C;

                C.addr = tmp;
                C.type   = type(tmp);

                C.file   = file(tmp);
                C.line   = line(tmp);
                C.count  = count(tmp);

                C.ival   = ival(tmp);
                C.rval   = rval(tmp);
                C.fval   = fval(tmp);

                C.cdr    = cdr(tmp);

                ShadowStack[i][j] = C;
                }
            }
        }

    int scan = HeapBottom;
    while (scan <= MEMORY_SPOT)
        {
        if (ismarked(scan))
            {
            char* TYPE = type(scan);
            if (TYPE == CONS || TYPE == ARRAY)
                {
                /* Make sure we dont change pointers to 
                   objects that do not move
                 */
                int tmp = car(scan);
                if (tmp >= HeapBottom)
                    {
                    setcar(scan,fwd(tmp));
                    }
                tmp = cdr(scan);
                if (tmp >= HeapBottom)
                    {
                    setcdr(scan,fwd(tmp));
                    }
                }
            else if (TYPE == STRING)
                {
                int tmp = cdr(scan);
                if (tmp >= HeapBottom)
                    {
                    setcdr(scan,fwd(tmp));
                    }
                }
            setstatus(scan,UPDATED);
            }
        ++scan;
        }
    MEMORY_SPOT = NEW_MEM_SPOT;
    }

#define move(scan,dest)                                         \
    do{                                                         \
    settype(dest,type(scan));                                   \
                                                                \
    setline(dest,line(scan));                                   \
    setfile(dest,file(scan));                                   \
    setcount(dest,count(scan));                                 \
                                                                \
    setival(dest,ival(scan));                                   \
    setrval(dest,rval(scan));                                   \
    setfval(dest,fval(scan));                                   \
                                                                \
    if (StackDebugging)                                         \
        {                                                       \
        THE_CARS.creator[dest] = THE_CARS.creator[scan];        \
        THE_CARS.lastEditor[dest] = THE_CARS.lastEditor[scan];  \
        THE_CARS.lastFile[dest] = THE_CARS.lastFile[scan];      \
        THE_CARS.lastLine[dest] = THE_CARS.lastLine[scan];      \
        }                                                       \
                                                                \
    setcdr(dest,cdr(scan));                                     \
    }while(0);

inline void
relocate(int start,int end)
    {
    int scan = start;
    while (scan < end)
        {
        if (status(scan) == UPDATED)
            {
            move(scan, fwd(scan));
            setstatus(scan,UNMARKED);
            setfwd(scan,scan);
            }
        ++scan;
        }
    return;
    }

/* markRoots - mark all of the root objects and the children of root objects
 */

void
markRoots(int mode)
    {
    /* Marking Phase */
    int i,j;
    for(i = 0 ; i < CreatedThreads; ++i)
        {
        for (j = 0; j< StackSpot[i] ; ++j)
            {
                markObject(Stack[i][j],mode);
            }
        }
    }

/* markObject - marks a root list item and all its children, recursively
 *
 */

void
markObject(int obj,int mode)
    {
    if(status(obj) == mode)
        {
        return;
        }

    struct Stack *s = create_stack();
    if( s == 0)
        {
        Fatal( "Could not create the stack");
        }

    if( push(s,obj) == -1) 
        {
        Fatal("Could not push onto stack");
        }

    while( !empty(s) ) {

        int o = pop(s);

        /* Ignore all marked cells */
        if(status(o) == mode)
            {
            continue;
            }
        
        setstatus(o,mode);

        char* TYPE = type(obj);
        if (TYPE == ARRAY || TYPE == CONS)
            {
            if( push(s,(cdr(o))) == -1)
                {
                Fatal("Could not push to the stack");
                }

            if( push(s,(car(o))) == -1)
                {
                Fatal("Could not push to the stack");
                }
            }
        else if(TYPE == STRING)
            {
            push(s,(cdr(o)));
            }
        }
    delete_stack(s);
    }

/*
 *  scanFree - Searches memory for unmarked cells.  Scans from end first 
 *  to build up contigious memory, then scans from the bottom of the heap
 *  to build up the free list.
 *
 */

void
scanFree(void)
    {
    int spot;

    /* Rebuild contigious */
    spot = MEMORY_SPOT - 1;
    while( !ismarked(spot) )
        {
        if(StackDebugging)
            {
            creator(spot) = -1;
            }
        spot--;
        }
    MEMORY_SPOT = spot + 1;
    
    /* Rebuld freelist */
    for(spot = HeapBottom; spot < MEMORY_SPOT; ++spot)
        {
        if(!ismarked(spot))
            {
            freePush(spot);
            }
        else{
            setstatus(spot,UNMARKED);
            }
        }
    }

/* 
 *  rawCopy  -  Copy the cell from old to newMemorySpot.  After copy we
 *              then set the status of the new and old to MOVED and 
 *              UNMOVED respectivly.  The cdr in the old semispace now 
 *              points to the new location in the new semi-space.
 */

#define RAW_COPY(OLD)                                       \
    do {                                                    \
    int NEW = NEW_MEM_SPOT;                                 \
                                                            \
    status(OLD) = MOVED;                                    \
                                                            \
    newtype(NEW) =  type(OLD);                              \
                                                            \
    newline(NEW) =  line(OLD);                              \
    newfile(NEW) =  file(OLD);                              \
    newcount(NEW) = count(OLD);                             \
                                                            \
    newival(NEW) =  ival(OLD);                              \
    newrval(NEW) =  rval(OLD);                              \
    newfval(NEW) =  fval(OLD);                              \
                                                            \
    if (StackDebugging)                                     \
        {                                                   \
        NEW_CARS.creator[NEW]       = creator(OLD);         \
        NEW_CARS.lastEditor[NEW]    = lastEditor(OLD);      \
        NEW_CARS.lastFile[NEW]      = lastFile(OLD);        \
        NEW_CARS.lastLine[NEW]      = lastLine(OLD);        \
        }                                                   \
                                                            \
    newstatus(NEW) = UNMOVED;                               \
                                                            \
    newcdr(NEW) = cdr(OLD);                                 \
                                                            \
    cdr(OLD) = NEW;                                         \
                                                            \
    ++NEW_MEM_SPOT;                                         \
    }while(0)

/* moveBackbone - move the backbone of an array or string
 *              - this routine handles receiving a pointer to
 *                the middle of a string or array
 */

int
moveBackbone(int old)
    {

    /* symbols don't have backbones */
    if(old < HeapBottom)
        {
        return old;
        }

    /* If we have already been copied over then return forwarding index */
    if(status(old) == MOVED)
        {
        return cdr(old);
        }

    char* t = type(old);

    /* Single CELL backbones */
    if (t != STRING && t != ARRAY)   /* CONS, SYMBOL, INTEGER, REAL */
        {
        RAW_COPY(old);
        }
    else   /* STRING or ARRAY */
        {
        /* 
            DOCUMENTATION:
                This should copy the ENTIRE array or 
                string structure over (not contents 
                of array). 
        */

        ASSERT(t == STRING || t == ARRAY);

        int orig = old;     // Need to this to restore old
        int size = count(old);

        /* Backup to the beginning of the array/string */
        while (type(old-1) == t && count(old-1) == size + 1)
            {
            --old;
            ++size;
            }

        /* Move ALL the cells in the string/array */
        while(size > 0)
            {
            ASSERT( t == type(old));
            RAW_COPY(old);  // Updates memory spot
            newcdr(NEW_MEM_SPOT - 1) = NEW_MEM_SPOT;
            ++old;
            --size;
            }
        /* reset the last cdr pointer to nil */
        newcdr(NEW_MEM_SPOT - 1) = 0;

        old = orig;
        }

    return cdr(old);
    }

/* 
 *  moveStackItem   - Move what was at the old cars position to the space at newMemorySpot
 *               - via moveX helper functions, transfers cars and cdrs, if necessary
 *               - returns the new memory location
 *
 */

int
moveStackItem(int old)
    {
    /* Symbols have already been moved and they are at the */
    /* same (relative) address */
    if(old < HeapBottom)
        {
        return old;
        }

    /* Already moved */
    if(status(old) == MOVED) 
        {
        return cdr(old); /* cdr holds the forwarding address */
        }

    /* Starting position */
    int spot = NEW_MEM_SPOT;

    /* Shallow copy */
    (void) moveBackbone(old); // Where old was placed


    /* 
        DOCUMENTATION : 
            Why can't I just check for a CON or ARRAY and handle each one?
            If I do it throws a segmentation fault for some unknown reason.

        BECAUSE:
            When we copy over items we need to scan them and make sure they 
            are not arrays.  So what this does is copies over items, scans 
            over them and if they are an array we make sure we copy the 
            contents of the array, the same for a cons.
    */

    /* deep copy */
    while(spot < NEW_MEM_SPOT)
        {

        char *t = newtype(spot);
        /* If we have not performed a deep copy on this node */
        if(t == ARRAY)
            {
            /* newcar(spot) points to a location in theCars */
            newcar(spot) = moveBackbone(newcar(spot));
            }
        else if(t==CONS)
            {
            /* newcar(spot) points to a location in theCars */
            newcar(spot) = moveBackbone(newcar(spot));

            /* newcdr(spot) points to a location in theCars */
            newcdr(spot) = moveBackbone(newcdr(spot));
            }
        spot++;
        }
    return cdr(old);
    }


/*
 * StopAndCopy - performs the semispace copying collection algorithm
 *
 * Premise - Two heaps are maintained.  One is the working heap,
 * the other will become the working heap once a garbage collection occurs.
 *
 */

static void 
StopAndCopy(void)
    {
    int i,j;

    /* Move Qeueue Items  */
    if(StackDebugging)
        {
        for(i = HeapBottom;i < MEMORY_SPOT; ++i)
            {
            if(creator(i) == -1)
                {
                printf("Error at position %d\n",i);
                debug("Cell",i);
                Fatal("allocated cell not attached to a thread\n");
                }
            }
        }

    for (i = 1; i < CreatedThreads; ++i)
        {
        /* If not used then env and expr are 0 */
        ThreadQueue[i].env = moveStackItem(ThreadQueue[i].env);
        ThreadQueue[i].expr = moveStackItem(ThreadQueue[i].expr);
        }

    /* Move Stack Items */
    for (i = 0; i < CreatedThreads; ++i)
        {
        for (j = 0; j < StackSpot[i]; ++j)
            {
            char *oldType = type(Stack[i][j]);
            Stack[i][j] = moveStackItem(Stack[i][j]);
            char *newType = newtype(Stack[i][j]);

            // The types better not be different
            if (oldType != newType)
               {
               printf("Old type: %s\n", oldType); 
               printf("New type: %s\n", type(Stack[i][j])); 
               Fatal("After moving a stack item, the types changed...\n");
               }

            if(StackDebugging)
                {
                int tmp = Stack[i][j];

                S_CELL C;

                C.addr = tmp;

                C.type   = type(tmp);

                C.file   = file(tmp);
                C.line   = line(tmp);
                C.count  = count(tmp);

                C.ival   = ival(tmp);
                C.rval   = rval(tmp);
                C.fval   = fval(tmp);

                C.cdr    = cdr(tmp);

                ShadowStack[i][j] = C;
                }
            }
        }   

    /* flip the semispace with the working heap */
    CELL tempCars = THE_CARS;
    THE_CARS = NEW_CARS;
    NEW_CARS = tempCars;

    MEMORY_SPOT = NEW_MEM_SPOT;
    NEW_MEM_SPOT = HeapBottom;
    /*
        DOCUMENTATION : 
            Is this correct?  From the above 4 lines I assume everything is 
            swapped and I should be able to set this back to nothing.
     */

    // Clear the data, for debugging purposes
    int count = MemorySize - HeapBottom;
    if(StackDebugging)
        {
        memset(NEW_CARS.creator + HeapBottom,-1, sizeof(int) * count);
        memset(THE_CARS.creator + MEMORY_SPOT,-1, sizeof(int) * (MemorySize-MEMORY_SPOT));
        }
    memset(NEW_CARS.type    + HeapBottom,  0, sizeof(char*) * count);
    memset(NEW_CARS.ival    + HeapBottom, -1, sizeof(int)   * count);
    memset(NEW_CARS.cdr     + HeapBottom, -1, sizeof(int)   * count);
    memset(NEW_CARS.status  + HeapBottom,  0, sizeof(char)  * count);

    if (StackDebugging)
        {
        for (i = HeapBottom; i < MEMORY_SPOT; ++i)
            {
            NEW_CARS.creator[i] = -1;
            ASSERT(THE_CARS.creator[i] >= 0);
            }
        }
    }

void
saveMTStack(char *fname, int ext, int index)
    {
    FILE *f;
    int i;
    int last = 0;
    char buf[512];
    /* concatenate the filename with the extension */
    snprintf(buf,sizeof(buf),"%s%s%d", fname, ".", ext);
    printf("\nsaving stack to: %s\n",buf);

    /* open the filename in write mode */
    f = fopen(buf, "w");
    for (i = 0; i < STACK_SPOT; ++i)
        {
        fprintf(f,"%d: ",i);
        debugOut(f, 0, STACK[i]);
        if (isObject(STACK[i]))
            {
            last = STACK[i];
            ppToFile(f,0);
            //ppTable(STACK[i],0,0);
            }
        }

    while (env_context(last) != 0)
        last = env_context(last);

    fprintf(f,"The top level environment: ");
    ppToFile(f,0);
    ppTable(last,0,0);

    /* close the file */
    fflush(f);
    fclose(f);
    }

/* saveStack - saves the contents of the stack to a file with a numeric extension
 */

void
saveStack(char *fname, int ext)
    {
    saveMTStack(fname, ext, 0);
    fflush(stdout);
    }

/* diff - compares 2 files produced by saveStack.  If the files differ
 *        the program will exit with an error.  Otherwise, it will continue
 */

void
diff(char *first, char *second, int ext)
    {
    printf("\nStaring Diff\n");
    /* concatenate the first filename with extension */
    char *fnameOne = malloc((strlen(first) + 1) * sizeof(char) + sizeof(int));
    if(fnameOne==NULL)
        {
            Fatal("Diff : Out of memory, fnameOne");
        }

    sprintf(fnameOne, "%s%s%d", first, ".", ext);

    /* concatenate the second filename with extension */
    char *fnameTwo = malloc((strlen(second) + 1) * sizeof(char) + sizeof(int));
    if(fnameTwo==NULL)
        {
            Fatal("Diff : Out of memory, fnameTwo");
        }

    sprintf(fnameTwo, "%s%s%d", second, ".", ext);

    /* construct the diff command */
    char *cmd = malloc((6 + strlen(fnameOne) + strlen(fnameTwo)) * sizeof(char));
    if(cmd==NULL)
        {
            Fatal("DIff : Out of memory, fnameOne");
        }

    sprintf(cmd, "%s%s%s%s", "diff ", fnameOne, " ", fnameTwo);

    int res = system(cmd);

    /* free the buffers */
    free(cmd);
    free(fnameOne);
    free(fnameTwo);

    /* if the diff returns 0 then the files are the same */
    if (res != 0)
        {
        /* the diff failed! */
        Fatal("diff failed!\n");
        }
    printf("\nDiff Done!\n");
    }

void
printStack()
{
    int tid = THREAD_ID;
    int len = STACK_SPOT;
    int i;

    printf("Printing the stack for thread %d\n" , tid);

    //T_DATA T = Thread[tid];

    for(i = 0;i<len;i++)
    {
        S_CELL C = ShadowStack[tid][i];
        int addr = Stack[tid][i];

        char *st = type(addr);
        char *sh = C.type;

        if (st != sh)
            {
            printf("Diff : Stack[%d] = %s, Shadow[%d] = %s\n",
                    i,
                    st,
                    i,
                    sh);
            }
        if(StackDebugging)
            {
            printf("Thread = %d, Stack addr = %d, Creator %d, Last Editor = %d, Last Place Edited = %s, Last Line Edited = %d\n",
                    tid,
                    i,
                    creator(addr),
                    lastEditor(addr),
                    lastFile(addr),
                    lastLine(addr)
                   );
            }
    }
}

static double
getTime()
    {
    struct timeval tv;
    gettimeofday(&tv,(struct timezone *)0);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
    }


void
print_stack()
{
    int i;
    printf("Stack (%d):\n", STACK_SPOT);
    for(i = 0 ; i < STACK_SPOT; ++i) {
        debug("\t",STACK[i]);
    }
}
