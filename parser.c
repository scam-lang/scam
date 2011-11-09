#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <assert.h>
#include "cell.h"
#include "types.h"
#include "parser.h"
#include "lexer.h"
#include "env.h"
#include "util.h"

extern void debug(char *,int);

/* recursive descent parsing function */

static int exprSeq(PARSER *);
static int expr(PARSER *);

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

    if (p->input == 0)
        {
        freeParser(p);
        return 0;
        }

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
parse(PARSER *p)
    {
    int result,end;

    //printf("starting to parse...\n");
    if (check(p,END_OF_INPUT)) return 0;

    result = exprSeq(p);
    rethrow(result,0);
    push(result);
    end = match(p,END_OF_INPUT);
    result = pop();
    rethrow(end,0);

    assureMemory("parse",1,&result,(int *)0);
    result = uconsfl(beginSymbol,result,file(result),line(result));
    //printf("done parsing.\n");
    return result;
    }

/* iterative version */

static int
exprSeq(PARSER *p)
    {
    int e,b;
    int head,hook;

    e = expr(p);
    rethrow(e,0);
    head = ucons(e,0);
    hook = head;
    //debug("head",head);

    push(head);
    push(hook);
    while (isExprPending(p))
        {
        b = expr(p);
        rethrow(b,2);
        assureMemory("exprSeq",1,&b,(int *)0);
        hook = pop();
        cdr(hook) = ucons(b,0);
        hook = cdr(hook);
        push(hook);
        }

    hook = pop();
    head = pop();

    return head;
    }

/* recursive version

static int
exprSeq(PARSER *p)
    {
    int e,b,result;

    printf("in exprSeq...\n");
    e = expr(p);
    rethrow(e,0);
    debug("exprSeq, e",e);

    push(e);
    if (isExprSeqPending(p))
        b = exprSeq(p);
    else
        b = 0;
    e = pop();
    rethrow(b,0);
    debug("exprSeq, b",b);

    result = ucons(e,b);
    debug("exprSeq, result",result);
    printf("leaving exprSeq.\n");
    if (gccount >= 1)
        {
        printf("cells left: %d\n", MemorySize - MemorySpot);
        //getchar();
        }
    return result;
    }
*/

static int
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
        rethrow(r,0);
        assureMemory("expr:quote",2,&r,(int *)0);
        result = uconsfl(quoteSymbol,ucons(r,0),fi,li);
        }
    else if (check(p,BACKQUOTE))
        {
        q = match(p,BACKQUOTE);
        fi = file(q);
        li = line(q);
        r = expr(p);
        rethrow(r,0);
        assureMemory("expr:backquote",2,&r,(int *)0);
        result = uconsfl(backquoteSymbol,ucons(r,0),fi,li);
        }
    else if (check(p,COMMA))
        {
        q = match(p,COMMA);
        fi = file(q);
        li = line(q);
        r = expr(p);
        rethrow(r,0);
        assureMemory("expr:comma",2,&r,(int *)0);
        result = uconsfl(commaSymbol,ucons(r,0),fi,li);
        }
    else if (check(p,OPEN_PARENTHESIS))
        {
        q = match(p,OPEN_PARENTHESIS);
        fi = file(q);
        li = line(q);
        if (isExprSeqPending(p))
            result = exprSeq(p);
        else
            result = 0;
        rethrow(result,0);
        rethrow(match(p,CLOSE_PARENTHESIS),0);
        if (result != 0)
            {
            file(result) = fi;
            line(result) = li;
            }
        }
    else if (isThrow(p->pending))
        return p->pending;
    else
        {
        assureMemory("expr:throw",1000,(int *)0);
        return throw(syntaxExceptionSymbol,
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
            assureMemory("match:throw",1000,(int *)0);
            return throw(syntaxExceptionSymbol,
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

    //printf("type(p->pending) is %s\n",type(p->pending)); //if (type(p->pending) == ID) ppf("pending is ",p->pending,"\n");

    if (isThrow(p->pending))
        return 0;
    else
        return type(p->pending) == t;
    }

extern char *Home;

static FILE *
openScamFile(char *fileName)
    {
    char buffer[512];
    FILE *fp;

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
