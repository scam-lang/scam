
/*
 *  Main Author : John C. Lusth
 *  Added Header : Jeffrey Robinson
 *  Last Modified : May 4, 2014
 *
 *  TODO : Description
 *
 */

#include <stdlib.h>

#include "scam.h"
#include "lexer.h"
#include "cell.h"
#include "types.h"
#include "env.h"

/* pending functions */

static int isExprSeqPending(PARSER *);
static int isExprPending(PARSER *);

/* parser support */

static int check(PARSER *,char *);
static int match(PARSER *,char *);
static FILE *openScamFile(char *);

/*
 
//scam grammar
//
//   TERMINALS are in (mostly) uppercase
//   nonTerminals are in (mostly) lowercase
//   *empty* is epsilon

//START RULE

    exprSeq    : expr 
               | expr exprSeq
               ;

    expr       : INTEGER
               | REAL
               | STRING
               | ID
               | OPAREN opt-expr CPAREN
               | QUOTE expr
               | COMMA expr
               ;

*/

/* this is the function that intefaces the parser with other modules */

PARSER *
newParser(char *fileName)
    {
    PARSER *p = (PARSER *) malloc(sizeof(PARSER));
    p->pending = -1;
    p->line = 1;
    p->file = findSymbol(fileName);
    p->pushedBack = 0;
    p->input = openScamFile(fileName);
    p->output = stdout;
    p->buffer = 0;
    p->bufferIndex = 0;

    if (p->input == 0)
        {
        freeParser(p);
        return 0;
        }

    return p;
    }

/* this is the function that intefaces the parser with other modules */

PARSER *
newParserFP(FILE *fp,char *fileName)
    {
    PARSER *p = (PARSER *) malloc(sizeof(PARSER));
    p->pending = -1;
    p->line = 1;
    p->file = findSymbol(fileName);
    p->pushedBack = 0;
    p->input = fp;
    p->output = stdout;
    p->buffer = 0;
    p->bufferIndex = 0;

    return p;
    }

void
freeParser(PARSER *p)
    {
    if (p->input != 0 && p->input != stdin)
        fclose(p->input);

    if (p->output != 0 && p->output != stdout)
        fclose(p->output);

    free(p);
    }

int
scamParse(PARSER *p)
    {
    int result,end;

    if (check(p,END_OF_INPUT)) 
        {
        return 0;
        }

    result = exprSeq(p);
    rethrow(result);
    PUSH(result);
    end = match(p,END_OF_INPUT);
    result = POP();
    rethrow(end);
    //printf("back from parsing\n");
    P();
    ENSURE_MEMORY(1,&result,(int *) 0);
    result = cons2(BeginSymbol,result);
    V();
    return result;
    }

/* iterative version */

int
exprSeq(PARSER *p)
    {
    int e,b;
    int head,hook;

    e = expr(p);
    rethrow(e);
    
    P();
    ENSURE_MEMORY(1,&e,(int *) 0);
    head = cons(e,0);
    V();
    hook = head;
    //debug("head",head);

    PUSH(head);
    PUSH(hook); /* peek index 0 */
    while (isExprPending(p))
        {
        b = expr(p);
        rethrowPop(b,2);
        P();
        int _tmp = cons(b , 0 );
        V();
        hook = PEEK(0);
        cdr(hook) = _tmp;
        REPLACE(0,cdr(hook));
        /* STACKCHECK */
        /* UPDATE(1); */
        }
    (void) POP(); /* hook */
    head = POP();

    return head;
    }

int
expr(PARSER *p)
    {
    int r,q;
    int fi,li;
    int result;

    if (check(p,INTEGER))
        result = match(p,INTEGER);
    else if (check(p,REAL))
        result = match(p,REAL);
    else if (check(p,STRING))
        result = match(p,STRING);
    else if (check(p,SYMBOL))
        result = match(p,SYMBOL);
    else if (check(p,QUOTE))
        {
        q = match(p,QUOTE);
        fi = file(q);
        li = line(q);
        r = expr(p);
        rethrow(r);
        P();
        ENSURE_MEMORY(2,&r,(int *) 0);
        result = consfl(QuoteSymbol,cons(r,0),fi,li);
        V();
        }
    else if (check(p,BACKQUOTE))
        {
        q = match(p,BACKQUOTE);
        fi = file(q);
        li = line(q);
        r = expr(p);
        rethrow(r);
        P();
        ENSURE_MEMORY(2,&r,(int *) 0);
        result = consfl(BackquoteSymbol,cons(r,0),fi,li);
        V();
        }
    else if (check(p,COMMA))
        {
        q = match(p,COMMA);
        fi = file(q);
        li = line(q);
        r = expr(p);
        rethrow(r);
        P();
        ENSURE_MEMORY(2,&r,(int *) 0);
        result = consfl(CommaSymbol,cons(r,0),fi,li);
        V();
        }
    else if (check(p,OPEN_PARENTHESIS))
        {
        q = match(p,OPEN_PARENTHESIS);
        fi = file(q);
        li = line(q);
        if (isExprSeqPending(p))
            {
            result = exprSeq(p);
            rethrow(result);
            file(result) = fi;
            line(result) = li;
            }
        else
            result = 0;


        PUSH(result);
        q = match(p,CLOSE_PARENTHESIS);
        rethrowPop(q,1);
        result = POP();
        }
    else if (isThrow(p->pending))
        return p->pending;
    else
        {
        return throw(EmptyExpressionSymbol,
            "file %s,line %d: expected an expression, got %s instead",
            SymbolTable[p->file],p->line,type(p->pending));
        }

    return result;
    }

/***** pending utilities ************************************************/

static int
isExprSeqPending(PARSER *p)
    {
    return isExprPending(p);
    }

static int
isExprPending(PARSER *p)
    {
    return check(p,INTEGER) || check(p,REAL) || check(p,STRING)
        || check(p,OPEN_PARENTHESIS) || check(p,SYMBOL)
        || check(p,QUOTE) || check(p,BACKQUOTE) || check(p,COMMA);
    }

/***** parser utilities ************************************************/

static int
match(PARSER *p,char *t)
    {
    int old;
    int matches;

    matches = check(p,t);
    if (!matches)
        {
        if (isThrow(p->pending))
            return p->pending;
        else
            {
            return throw(SyntaxExceptionSymbol,
                "file %s,line %d: expecting %s, found %s instead",
                SymbolTable[p->file],p->line,t,type(p->pending));
            }
        }

    old = p->pending;
    p->pending = -1;
    return old;
    }

static int
check(PARSER *p,char *t)
    {
    if (p->pending == -1)
        {
        //printf("about to lex...\n");
        p->pending = lex(p);
        //printf("token is %s\n",type(p->pending));
        }

    //printf("type(p->pending) is %s\n",type(p->pending)); //if (type(p->pending) == ID) debug("pending is",p->pending);

    if (isThrow(p->pending))
        return 0;
    else
        return type(p->pending) == t;
    }

static FILE *
openScamFile(char *fileName)
    {
    char buffer[512];
    FILE *fp;

    // null fileName means stdin

    if (fileName == 0)
        return stdin;

    //printf("looking for file in current directory\n");

    fp = fopen(fileName,"r");
    if (fp != 0) return fp;

    snprintf(buffer,sizeof(buffer),"%s/scam/%s",Home,fileName);
    //printf("looking for file in %s\n",buffer);
    fp = fopen(buffer,"r");
    if (fp != 0) return fp;
    
    snprintf(buffer,sizeof(buffer),"/usr/local/lib/scam/%s",fileName);
    //printf("looking for file in %s\n",buffer);
    fp = fopen(buffer,"r");
    if (fp != 0) return fp;
    
    snprintf(buffer,sizeof(buffer),"/usr/lib/scam/%s",fileName);
    //printf("looking for file in %s\n",buffer);
    fp = fopen(buffer,"r");
    if (fp != 0) return fp;

    return 0;
    }
