#define TRACE_SAVES 0
#define CHECK_SAVES 0

typedef struct cellobj
    {
    char *type;
    int line;
    int file;
    int count;

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
#define car(a)      (the_cars[a].ival)
#define caar(a)     (the_cars[the_cars[a].ival].ival)
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

/* top level helpers */

extern void memoryInit(int);

/* lexing helpers */

extern int newString(char *);
extern int newSymbol(char *);
extern int newInteger(int);
extern int newReal(double);
extern int newPunctuation(char *);

extern char **SymbolTable;
extern int MaxSymbols;
extern int findSymbol(char *);

extern int zero;
extern int one;
extern int contextSymbol;
extern int codeSymbol;
extern int dynamicContextSymbol;
extern int thisSymbol;
extern int parametersSymbol;
extern int thunkSymbol;
extern int nameSymbol;
extern int closureSymbol;
extern int builtInSymbol;
extern int constructorSymbol;
extern int objectSymbol;
extern int typeSymbol;
extern int valueSymbol;
extern int traceSymbol;
extern int throwSymbol;
extern int quoteSymbol;
extern int anonymousSymbol;
extern int lambdaSymbol;
extern int dollarSymbol;
extern int atSymbol;
extern int uninitializedSymbol;
