#define TRACE_SAVES 0
#define CHECK_SAVES 0

typedef struct cellobj
    {
    char *type;
    int line;
    int file;
    int count;
    int transferred;

    int ival;
    double rval;
    int (*fval)(void);
    } CELL;

extern int MemorySpot;
extern int MemorySize;
extern int CodeSize;

extern CELL *the_cars;
extern int *the_cdrs;

extern CELL *new_cars;
extern int *new_cdrs;

extern int LineNumber;
extern int FileIndex;

extern int cons(int,int);
extern int ucons(int,int);
extern int length(int);

#define cfile (symbols[FileIndex])
#define cline (LineNumber)

#define type(a)     (the_cars[a].type)
#define ival(a)     (the_cars[a].ival)
#define rval(a)     (the_cars[a].rval)
#define fval(a)     (the_cars[a].fval)
#define count(a)    (the_cars[a].count)
#define file(a)     (the_cars[a].file)
#define filename(a) (symbols[the_cars[a].file])
#define line(a)     (the_cars[a].line)
#define transferred(a)     (the_cars[a].transferred)

#define car(a)      (the_cars[a].ival)
#define caar(a)     (car(car(a)))
#define cdar(a)     (cdr(car(a)))
#define cdr(a)      (the_cdrs[a])
#define cddr(a)     (the_cdrs[the_cdrs[a]])
#define cdddr(a)    (the_cdrs[the_cdrs[the_cdrs[a]]])
#define cddddr(a)   (the_cdrs[the_cdrs[the_cdrs[the_cdrs[a]]]])
#define cdddddr(a)  (the_cdrs[the_cdrs[the_cdrs[the_cdrs[the_cdrs[a]]]]])
#define cadr(a)     (car(cdr(a)))
#define caddr(a)    (car(cddr(a)))
#define cadddr(a)   (car(cdddr(a)))
#define caddddr(a)  (car(cddddr(a)))
#define cadddddr(a) (car(cdddddr(a)))

#define line(a) (the_cars[a].line)

#define sameSymbol(a,b) (type(a) == SYMBOL && ival(a) == ival(b))

//#define push(x) (rootList = cons(x,rootList))

/* top level helpers */

extern void memoryInit(int);
extern int pop(void);
extern void push(int);
extern int StackPtr;

/* lexing helpers */

extern int newString(char *);
extern int newSymbol(char *);
extern int newSymbolFromIndex(int);
extern int newInteger(int);
extern int newReal(double);
extern int newPunctuation(char *);

extern char **SymbolTable;
extern int SymbolCount;
extern int MaxSymbols;
extern int findSymbol(char *);

extern char *cellString(char *,int,int);
extern char *cellStringTr(char *,int,int);

extern int rootList;
extern void assureMemory(char *,int,int *,...);

extern int zero;
extern int one;
extern int contextSymbol;
extern int codeSymbol;
extern int thisSymbol;
extern int parametersSymbol;
extern int thunkSymbol;
extern int nameSymbol;
extern int envSymbol;
extern int closureSymbol;
extern int builtInSymbol;
extern int constructorSymbol;
extern int objectSymbol;
extern int typeSymbol;
extern int labelSymbol;
extern int valueSymbol;
extern int traceSymbol;
extern int throwSymbol;
extern int quoteSymbol;
extern int anonymousSymbol;
extern int lambdaSymbol;
extern int dollarSymbol;
extern int atSymbol;
extern int sharpSymbol;
extern int ampersandSymbol;
extern int hatSymbol;
extern int uninitializedSymbol;
extern int errorSymbol;
extern int beginSymbol;
extern int trueSymbol;
extern int falseSymbol;
extern int backquoteSymbol;
extern int commaSymbol;
extern int inputPortSymbol;
extern int outputPortSymbol;
extern int eofSymbol;
extern int elseSymbol;
extern int nilSymbol;
extern int fileSymbol;
extern int lineSymbol;
extern int messageSymbol;

extern int readIndex;
extern int writeIndex;
extern int appendIndex;
extern int stdinIndex;
extern int stdoutIndex;
