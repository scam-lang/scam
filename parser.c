#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <assert.h>
#include "cell.h"
#include "types.h"
#include "lexer.h"
#include "parser.h"
#include "util.h"

/* operator types */

FILE *Input;
FILE *Output;

/* the pending input token, -1 is not assigned */

int Pending = -1;

/* recursive descent parsing function */

static int exprSeq(void);
static int expr(void);

/* pending functions */

static int isExprSeqPending(void);
static int isExprPending(void);

/* parser support */

static int check(char *);
static int match(char *);

void
parserInit(char *fn)
    {
    LineNumber = 1;

    Input = OpenFile(fn,"r");
    FileIndex = findSymbol(fn);

    Output = stdout;
    }

/*
 
//xcheme Grammar
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

int
parse()
    {
    int result;
    Pending = -1;

    result = exprSeq();
    match(END_OF_INPUT);

    return cons(newSymbol("begin"),result);
    }

static int
exprSeq()
    {
    int e,b;

    e = expr();

    if (isExprSeqPending())
        b = exprSeq();
    else
        b = 0;

    return cons(e,b);
    }

static int
expr()
    {
    int p,q;
    int result;

    if (check(INTEGER))
        result = match(INTEGER);
    else if (check(REAL))
        result = match(REAL);
    else if (check(STRING))
        result = match(STRING);
    else if (check(SYMBOL))
        result = match(SYMBOL);
    else if (check(QUOTE) || check(BACKQUOTE) || check(COMMA))
        {
        p = match(QUOTE);
        q = expr();
        result = cons(p,cons(q,0));
        }
    else if (check(OPEN_PARENTHESIS))
        {
        result = exprSeq();
        match(CLOSE_PARENTHESIS);
        }
    else
        Fatal("syntax error on line %d\n",LineNumber);

    return result;
    }

/***** pending utilities ************************************************/

static int
isExprSeqPending()
    {
    return isExprPending();
    }

static int
isExprPending()
    {
    return check(INTEGER) || check(REAL) || check(STRING) || check(OPEN_PARENTHESIS)
        || check(SYMBOL) || check(QUOTE) || check(BACKQUOTE) || check(COMMA);
    }

/***** parser utilities ************************************************/

static int
match(char *t)
    {
    int old;

    if (!check(t))
        {
        Fatal("expecting %s, found %s instead",t,type(Pending));
        }

    old = Pending;
    Pending = -1;
    return old;
    }

static int
check(char *t)
    {
    if (Pending == -1)
        {
        //printf("about to lex...\n");
        Pending = lex();
        //printf("token is %s\n",type(Pending));
        }

    //printf("type(Pending) is %s\n",type(Pending));
    //if (type(Pending) == ID) ppf("Pending is ",Pending,"\n");

    return type(Pending) == t;
    }
