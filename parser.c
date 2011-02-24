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
#include "util.h"

/* operator types */

FILE *Input;
FILE *Output;

/* the pending input token, -1 is not assigned */

int Pending = -1;

/* recursive descent parsing function */

static int exprSeq(PARSER *);
static int expr(PARSER *);

/* pending functions */

static int isExprSeqPending(PARSER *);
static int isExprPending(PARSER *);

/* parser support */

static int check(PARSER *,char *);
static int match(PARSER *,char *);

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
               | OPAREN program CPAREN
               | QUOTE expr
               | COMMA expr
               ;

*/

/* this is the function that interaces the parser with other modules */

PARSER *
newParser(char *fileName)
    {
    PARSER *p = (PARSER *) malloc(sizeof(PARSER));
    p->pending = -1;
    p->line = 1;
    p->file = findSymbol(fileName);
    p->input = OpenFile(fileName,"r");
    p->output = stdout;

    return p;
    }

int
parse(PARSER *p)
    {
    int result;

    result = exprSeq(p);
    match(p,END_OF_INPUT);

    return cons(beginSymbol,result);
    }

static int
exprSeq(PARSER *p)
    {
    int e,b;

    e = expr(p);

    if (isExprSeqPending(p))
        b = exprSeq(p);
    else
        b = 0;

    return cons(e,b);
    }

static int
expr(PARSER *p)
    {
    int r;
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
        match(p,QUOTE);
        r = expr(p);
        result = cons(quoteSymbol,cons(r,0));
        }
    else if (check(p,BACKQUOTE))
        {
        match(p,BACKQUOTE);
        r = expr(p);
        result = cons(backquoteSymbol,cons(r,0));
        }
    else if (check(p,COMMA))
        {
        match(p,COMMA);
        r = expr(p);
        result = cons(commaSymbol,cons(r,0));
        }
    else if (check(p,OPEN_PARENTHESIS))
        {
        int f,l;
        match(p,OPEN_PARENTHESIS);
        f = p->file;
        l = p->line;
        result = exprSeq(p);
        match(p,CLOSE_PARENTHESIS);
        file(result) = f;
        line(result) = l;
        }
    else
        Fatal("syntax error on line %d\n",LineNumber);

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

    if (!check(p,t))
        {
        Fatal("expecting %s, found %s instead",t,type(p->pending));
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
        //printf("token is %s\n",type(Pending));
        }

    //printf("type(Pending) is %s\n",type(Pending));
    //if (type(Pending) == ID) ppf("Pending is ",Pending,"\n");

    return type(p->pending) == t;
    }
