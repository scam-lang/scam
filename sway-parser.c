
/*
 *  Main Author : John C. Lusth
 *  Added Header : Jeffrey Robinson
 *  Last Modified : May 4, 2014
 *
 *  TODO : Description
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <assert.h>

#include "scam.h"
#include "lexer.h"
#include "cell.h"
#include "types.h"
#include "env.h"
#include "util.h"

#define TRACE 0

/* operator types */

#define UPDATER              0       /* assign           */
#define LOGICAL_CONNECTIVE   1       /* AND, OR, and NOT */
#define LOGICAL_COMPARISON   2       /* <, >, and so on  */
#define ARITHMETIC           3       /* +, -, and so on  */
#define SELECT               4       /* . and [          */
#define NOTanOP              5

/* recursive descent parsing function */

static int functionDef(PARSER *);
static int varDef(PARSER *);
static int varDefList(PARSER *);
static int varDefItem(PARSER *);
static int optParamList(PARSER *);
static int paramList(PARSER *);
static int nakedBlock(PARSER *);
static int block(PARSER *);
static int definition(PARSER *);
static int statement(PARSER *);
static int sexpr(PARSER *);
static int exprAssign(PARSER *);
static int exprConnect(PARSER *);
static int exprCompare(PARSER *);
static int exprMath(PARSER *);
static int exprCall(PARSER *,int);
static int exprSelect(PARSER *,int);
static int primary(PARSER *);
static int optArgList(PARSER *);
static int argList(PARSER *);
static int extraArgs(PARSER *);
static int optExtraArgs(PARSER *);

/* pending functions */

static int isDefinitionPending(PARSER *p);
static int isStatementPending(PARSER *);
static int isBlockPending(PARSER *);
static int isExprPending(PARSER *);
static int opType(PARSER *);
static int operatorType(int);
static int isXCall(int);
static int hasDefinitions(int);

/* parser support */

static int check(PARSER *,char *);
static int match(PARSER *,char *);

/*

//Sway Grammar
//
//   TERMINALS are in (mostly) uppercase
//   nonTerminals are in (mostly) lowercase
//   *empty* is epsilon

//START RULE

    nakedBlock : *empty*
               | statement nakedBlock
               | definition nakedBlock
               ;

    definition : functionDef
               | varDef
               ;

    functionDef : FUNCTION OPAREN optParamList block
                ;

    varDef : VAR varDefList
           ;

    varDefList : varDefItem
               | varDefItem COMMA varDefList
               ;

    varDefItem : SYMBOL
               | VAR SYMBOL SYMBOL[=] expr
               ;

    optParamList : *empty*
                 | paramList
                 ;

    paramList : SYMBOL
              | SYMBOL COMMA
              | SYMBOL COMMA paramList
              ;

//BLOCKS and STATEMENTS

    block : OBRACE nakedBlock CBRACE

    statement :  expr SEMI
              ;

//EXPRESSIONS

    expr : exprAssign
         ;

    //Note: precedence is achieved by levels of expressions;
    //      earlier rules are at lower precedence

    exprAssign : exprConnect
               | exprConnect SYMBOL[=,tail=,head=] exprAssign
               ;

    //Note: exprAssign is left associative, the others right associative

    exprConnect : exprCompare [SYMBOL[&&,||] exprConnect]*
                ;

    exprCompare : exprMath [SYMBOL[<,<=,==,>=,>,!=] exprMath]*
                ;

    exprMath : exprCall (SYMBOL exprCall)*
             ;

    exprCall : exprSelect
             | exprSelect OPAREN optArgList CPAREN optExtraArgs
             | exprSelect OPAREN optArgList CPAREN optExtraArgs SYMBOL[.] exprCall
             ;

    //Note: a block that immediately follows a call is considered another
    //      argument to the call; optExtraArgs matches these blocks

    exprSelect : primary (SYMBOL[.] primary)*
               | primary (OBRACKET expr CBRACKET)*
               ;

    primary : INTEGER | REAL | STRING
            | SYMBOL
            | SYMBOL[return] expr
            | QUOTE SYMBOL
            | parenExpr
            | functionExpr
            | block
            ;

    //Note: return parses as a function call

    functionExpr : FUNCTION OPAREN optParamList CPAREN block
                 ;

    parenExpr : OPAREN expr CPAREN
              ;

    optArgList : *empty*
               | argList
               ;

    argList : expr
            | expr COMMA
            | expr COMMA argList
            ;

    optExtraArgs : extraArgs
                 | *empty*
                 ;

    extraArgs : block
              | block ELSE block
              | block ELSE exprSelect OPAREN optArgList CPAREN extrArgs
              ;

    //Note: extraArgs produces one (block) or two (block ELSE ...) extra
    //      arguments

*/

int
swayParse(PARSER *p)
    {
    int result,end;

    //printf("starting to parse...\n");
    if (check(p,END_OF_INPUT)) return 0;

    //printf("parsing now, %s pending\n",type(p->pending));
    result = nakedBlock(p);
    rethrow(result);
    PUSH(result); /* peek location is zero */

    end = match(p,END_OF_INPUT);
    rethrowPop(end,1);

    result = POP();
    P();
    ENSURE_MEMORY(1,&result,(int *)0); 
    result = cons2(BeginSymbol,result);
    V();
    return result;
    }

/* swayInteractive
 *
 * read a single item from the input
 *
 */

int
swayInteractive(PARSER *p)
    {
    //force a lex
    check(p,0);

    if (isDefinitionPending(p))
        return definition(p);
    else
        return statement(p);
    }

/*    definition : functionDef
 *               | varDef
 *               ;
 */

static int
definition(PARSER *p)
    {
    int d;
    int m;

    if (TRACE) printf("in definition...\n");

    if (check(p,SYMBOL) && SameSymbol(p->pending,VarSymbol))
        {
        d = varDef(p);
        rethrow(d);
        PUSH(d);
        m = match(p,SEMI);
        d = POP();
        rethrow(m);
        }
    else // function definition
        {
        d = functionDef(p);
        rethrow(d);
        }

    if (TRACE) debug("leaving definition",d);

    return d;
    }

/*
 *  functionDef : FUNCTION OptSymbol OPAREN optParamList block
 *              ;
 */

static int
functionDef(PARSER *p)
    {
    int f,v,params,body,result;
    int anonymous = 0;
    int m;

    if (TRACE) printf("in functionDef...\n");

    f = match(p,SYMBOL);
    rethrow(f);
    assert(SameSymbol(f,FunctionSymbol));

    if (check(p,SYMBOL))
        v = match(p,SYMBOL);
    else
        {
        v = newSymbol("anonymous");
        anonymous = 1;
        }

    PUSH(v);

    m = match(p,OPEN_PARENTHESIS);

    rethrowPop(m,1); /* v */

    params = optParamList(p);
    rethrowPop(params,1); /* v */

    PUSH(params);

    m = match(p,CLOSE_PARENTHESIS);
    rethrowPop(m,2); /* v and params */

    m = match(p,OPEN_BRACE);
    rethrowPop(m,2); /* v and params */

    //printf("about to parse function body\n");

    body = nakedBlock(p);

    rethrowPop(body,2); /* v and params */

    PUSH(body);

    m = match(p,CLOSE_BRACE);
    rethrowPop(m,3); /* v and params and body */

    if (anonymous)
        {
        m = match(p,SEMI);
        rethrowPop(m,3); /* v and params and body */
        }

    if (car(body) == 0)
        {
        POPN(3); /* v and params and body */
        return throw(SyntaxExceptionSymbol,
            "file %s, line %d: blocks must contain at least one statement",
            SymbolTable[p->file], p->line, type(p->pending));
        }

    //indent the name so that we can find the indent after the call

    P();
    ENSURE_MEMORY(3,(int *) 0);

    body = POP();
    params = POP();
    v = POP();

    result = cons(v,params);
    result = cons(result,body);
    result = cons2(DefineSymbol,result);
    V();

    if (TRACE) debug("leaving functionDef",result);

    return result;
    }
/*
 *  varDef : VAR varDefList
 *         ;
 */

static int
varDef(PARSER *p)
    {
    int v;
    int m;

    if (TRACE) printf("in varDef...\n");

    m = match(p,SYMBOL);
    rethrow(m);
    assert(SameSymbol(m,VarSymbol));
    v = varDefList(p);
    rethrow(v);
    //type(v) = VARIABLE_DEFINITION_LIST;

    if (cdr(v) == 0)
        {
        v = car(v);
        }
    else
        {
        P();
        ENSURE_MEMORY(1,&v,(int *) 0);
        v = cons2(BeginSymbol,v);
        V();
        }
    if (TRACE) debug("leaving varDef",v);
    return v;
    }

/*
 *  varDefList : varDefItem
 *             | varDefItem COMMA varDefList
 *             ;
 */

static int
varDefList(PARSER *p)
    {
    int item,result;

    if (TRACE) printf("in varDefList...\n");

    item = varDefItem(p);
    rethrow(item);
    PUSH(item);

    if (check(p,COMMA)) /* check is not gc-safe */
        {
        match(p,COMMA);
        result = varDefList(p);
        //item = restore("parser:varDefList:item");
        rethrow(result);
        }
    else
        {
        result = 0;
        }

    P();
    ENSURE_MEMORY(1,&result,(int *) 0);
    item = POP();

    result = cons(item, result);
    V();

    if (TRACE) debug("leaving varDefList",result);

    return result;
    }

/*    varDefItem : SYMBOL
 *              | VAR SYMBOL SYMBOL[=] expr
 *              ;
 */


static int
varDefItem(PARSER *p)
    {
    int v,init,result;

    if (TRACE) printf("in varDefItem...\n");

    v = match(p,SYMBOL);
    rethrow(v);

    PUSH(v);

    check(p,SYMBOL); // force a lex, if necessary (check is not gc-safe)

    if (SameSymbol(p->pending,EqSymbol))
        {
        match(p,SYMBOL); // ASSIGN
        init = sexpr(p);
        rethrow(init);
        P();
        ENSURE_MEMORY(3,&init,(int *) 0);
        v = POP();
        result = cons(init,0);
        result = cons(v,result);
        result = cons2(DefineSymbol,result);
        V();
        }
    else
        {
        if (TRACE > 1) printf("no initializer\n");
        P();
        ENSURE_MEMORY(2,(int *) 0);
        v = POP();
        result = cons(v,0);
        result = cons2(DefineSymbol,result);
        V();
        }


    if (TRACE) debug("leaving varDefItem",result);

    return result;
    }


/*    optParamList : *empty*
 *                | paramList
 *                ;
 */

static int
optParamList(PARSER *p)
    {
    if (check(p,SYMBOL))
        return paramList(p);
    else
        return 0;
    }

/*    paramList : SYMBOL
 *             | SYMBOL COMMA
 *             | SYMBOL COMMA paramList
 *             ;
 */

static int
paramList(PARSER *p)
    {
    int a,b;
    int result;

    if (TRACE) printf("in paramList...\n");

    a = match(p,SYMBOL);
    rethrow(a);
    PUSH(a);

    if (check(p,COMMA))
        {
        int m = match(p,COMMA);
        rethrowPop(m,1); /* a */
        if (check(p,SYMBOL))
            b = paramList(p);
        else
            b = 0;
        rethrowPop(b,1); /* a */
        }
    else
        {
        b = 0;
        }

    P();
    ENSURE_MEMORY(1,&b,(int *) 0);
    a = POP();
    result = cons(a,b);
    V();

    if (TRACE) debug("leaving paramList",result);

    return result;
    }

/*    nakedBlock : *empty*
 *                   | statement nakedBlock
 *                   | definition nakedBlock
 *                   ;
 */

static int
nakedBlock(PARSER *p)
    {
    int result;
    int item,others;

    if (TRACE) printf("in nakedBlock...\n");

    if (check(p,SYMBOL) && check(p,OPEN_BRACKET))
        {
        return throw(SyntaxExceptionSymbol,
            "file %s,line %d: expected an expression, got %s instead",
            SymbolTable[p->file],p->line,type(p->pending));
        }

    //debug("pending",p->pending);
    if (check(p,OPEN_BRACE))
        {
        return throw(SyntaxExceptionSymbol,
            "file %s,line %d: expected an expression, got a block instead",
            SymbolTable[p->file],p->line);
        }
    else if (isDefinitionPending(p))
        {
        item = definition(p);
        rethrow(item);
        PUSH(item);
        others = nakedBlock(p);
        rethrowPop(others,1); /* item */
        P();
        ENSURE_MEMORY(1,&others,(int *) 0);
        item = POP();
        result = cons(item,others);
        V();
        }
    else if (isStatementPending(p))
        {
        item = statement(p);
        rethrow(item);
        PUSH(item);
        others = nakedBlock(p);
        rethrowPop(others,1); /* item */
        P();
        ENSURE_MEMORY(1,&others,(int *) 0);
        item = POP();
        result = cons(item,others);
        V();
        }
    else
        {
        result = 0;
        }

    if (TRACE) debug("leaving nakedBlock",result);

    return result;
    }

/*    statement :  block
 *             | expr SEMI
 *             ;
 */

static int
statement(PARSER *p)
    {
    int r;
    int m;

    if (TRACE) printf("in statement...\n");

    r = sexpr(p);
    rethrow(r);
    if (TRACE > 1) printf("statement subtype is %s\n", type(r));

    if (!isXCall(r))
        {
        PUSH(r);
        m = match(p, SEMI);
        r = POP();
        rethrow(m);
        }
    else
        {
        r = cdr(r); // strip xcall tag
        }

    if (TRACE) debug("leaving statement",r);

    return r;
    }

/* block : OBRACE nakedBlock CBRACE */

static int
block(PARSER *p)
    {
    int b,m;

    match(p,OPEN_BRACE);
    b = nakedBlock(p);
    rethrow(b);
    PUSH(b);
    m = match(p,CLOSE_BRACE);
    rethrowPop(m,1); //for saved b
    P();
    ENSURE_MEMORY(1,(int *) 0);
    b = POP();

    if (hasDefinitions(b))
        b = cons2(ScopeSymbol,b);
    else
        b = cons2(BeginSymbol,b);
    V();
    return b;
    }

/* expr : exprAssign */

static int
sexpr(PARSER *p)
    {
    return exprAssign(p);
    }

/* exprAssign : exprConnect (SYMBOL(=,tail=,head=) exprConnect)*
 *
 * exprAssign is left associative
 */

static int
exprAssign(PARSER *p)
    {
    int a,b,c;
    int result = 0;

    if (TRACE) printf("in exprAssign...\n");

    a = exprConnect(p);
    rethrow(a);

    if (isXCall(a))
        {
        if (TRACE) debug("leaving exprAssign",a);
        return a;
        }

    if (opType(p) == UPDATER)
        {
        PUSH(a);
        b = match(p,SYMBOL);
        rethrowPop(b,1); /* a */
        PUSH(b);
        c = exprAssign(p);
        rethrowPop(c,2);
        P();
        ENSURE_MEMORY(3,&c,(int *) 0);
        b = POP();
        a = POP();

        result = cons(b,cons(a,cons(c,0)));
        V();
        }
    else
        result = a;

    if (TRACE) debug("leaving exprAssign",result);

    return result;
    }

/* exprConnect : exprCompare SYMBOL[&&,||] exprCompare)*
 *
 * exprConnect is left associative
 */

static int
exprConnect(PARSER *p)
    {
    int a,b,c;

    if (TRACE) printf("in exprConnect...\n");

    a = exprCompare(p);
    rethrow(a);

    if (isXCall(a))
        {
        if (TRACE) debug("leaving exprConnect",a);
        return a;
        }

    PUSH(a); /* peek location 0 */
    while (opType(p) == LOGICAL_CONNECTIVE)
        {
        int result;
        b = match(p,SYMBOL);
        rethrowPop(b,1); /* a */
        PUSH(b);
        c = exprCompare(p);
        rethrowPop(c,2);
        P();
        ENSURE_MEMORY(3,&c,(int *) 0);
        b = POP();
        a = PEEK(0);
        result = cons(b,cons(a,cons(c,0)));
        V();
        REPLACE(0,result);
        }

    a = POP();

    if (TRACE) debug("leaving exprConnect",a);

    return a;
    }

/* exprCompare : exprMath (SYMBOL[<,<=,==,!=,>=,>] exprMath)*
 *
 * exprCompare is left associative
 */

static int
exprCompare(PARSER *p)
    {
    int a,b,c;

    if (TRACE) printf("in exprCompare...\n");

    a = exprMath(p);
    rethrow(a);

    if (isXCall(a))
        {
        if (TRACE) debug("leaving exprCompare",a);
        return a;
        }

    PUSH(a);
    while (opType(p) == LOGICAL_COMPARISON)
        {
        int result;
        b = match(p,SYMBOL);
        rethrowPop(b,1); /* a */
        PUSH(b);
        c = exprMath(p);
        rethrowPop(c,2);
        P();
        ENSURE_MEMORY(3,&c,(int *) 0);
        b = POP();
        a = PEEK(0);
        result = cons(b,cons(a,cons(c,0)));
        V();
        REPLACE(0,result);
        }
    a = POP();

    if (TRACE) debug("leaving exprCompare",a);

    return a;
    }

/* exprMath : exprCall (SYMBOL[+,-,*,/,%,^] exprCall)* */

static int
exprMath(PARSER *p)
    {
    int a,b,c;
    int result = 0;

    if (TRACE) printf("in exprMath...\n");

    a = exprCall(p,0);
    rethrow(a);

    if (isXCall(a))
        {
        if (TRACE) debug("leaving exprMath",a);
        return a;
        }

    PUSH(a);
    while (opType(p) == ARITHMETIC)
        {
        int result;
        b = match(p,SYMBOL);
        rethrowPop(b,1); /* a */
        PUSH(b);
        c = exprCall(p,0);
        rethrowPop(c,2);
        P();
        ENSURE_MEMORY(3,&c,(int *) 0);
        b = POP();
        a = PEEK(0);
        result = cons(b,cons(a,cons(c,0)));
        V();
        REPLACE(0,result);
        }
    a = POP();

    if (TRACE) debug("leaving exprMath",result);

    return a;
    }

/* exprCall : exprSelect
 *          | exprSelect OPAREN optArgList CPAREN optExtraArgs
 *          | exprSelect OPAREN optArgList CPAREN optExtraArgs SYMBOL(.) exprCall
 */

static int
exprCall(PARSER *p,int item)
    {
    int op,args,extra;
    int result;
    int m;

    if (TRACE) printf("in exprCall...\n");

    if (item == 0)
        {
        op = exprSelect(p,0);
        }
    else
        {
        op = item;
        }

    rethrow(op);

    PUSH(op);

    if (check(p,OPEN_PARENTHESIS)) /* check is not gc-safe */
        {
        /* this is a function call */
        (void) match(p,OPEN_PARENTHESIS);
        args = optArgList(p);
        rethrowPop(args,1); /* for op */
        PUSH(args);
        m = match(p,CLOSE_PARENTHESIS);
        rethrowPop(m,2); /* for args and op */
        extra = optExtraArgs(p);
        rethrowPop(extra,2); /* for args and op */
        P();
        ENSURE_MEMORY(1,&extra,(int *) 0);
        args = POP();
        op = POP();
        /* append is destructive, does not use cons */
        result = cons(op,append(args,extra));
        V();
        if (extra != 0)
            {
            P();
            ENSURE_MEMORY(1,&result,(int *) 0);
            result = cons2(XcallSymbol,result);
            V();
            }
        if (opType(p) == SELECT)
            {
            result = exprSelect(p,result);
            result = exprCall(p,result);
            }
        }
    else
        {
        result = POP();  /* was op */
        }

    if (TRACE) debug("leaving exprCall",result);

    return result;
    }

/*  optExtraArgs : extraArgs
 *               | *empty*
 */

static int
optExtraArgs(PARSER *p)
    {
    if (isBlockPending(p))
        return extraArgs(p);
    else
        return 0;
    }

/*  extraArgs : block
 *            | block ELSE block
 *            | block ELSE exprSelect OPAREN optArgList CPAREN extrArgs
 */

static int
extraArgs(PARSER *p)
    {
    int b,c,result;

    b = block(p);
    rethrow(b);
    PUSH(b);

    check(p,0); /* force a lex (not gc-safe) */

    if (SameSymbol(ElseSymbol,p->pending))
        {
        (void) match(p,SYMBOL);
        if (isBlockPending(p))
            {
            c = block(p);
            rethrowPop(c,1); /* for b */
            P();
            ENSURE_MEMORY(2,&c,(int *) 0);
            b = POP();
            result = cons(b,cons(c,0));
            V();
            }
        else  /* parse an extended call */
            {
            int op,args,extra,m;
            op = exprSelect(p,0);
            rethrowPop(op,1); /* for b */
            PUSH(op);
            m = match(p,OPEN_PARENTHESIS);
            rethrowPop(m,2); /* for b and op */
            args = optArgList(p);
            rethrowPop(args,2); /* for b and op */
            PUSH(args);
            m = match(p,CLOSE_PARENTHESIS);
            rethrowPop(m,3); /* for b and op and args */
            extra = extraArgs(p);
            rethrowPop(extra,3); /* for b and op and args */

            P();
            ENSURE_MEMORY(3,&extra,(int *) 0);
            args = POP();
            op = POP();
            b = POP();

            /* append is destructive, does not use cons */
            result = cons(op,append(args,extra));
            result = cons(result,0);
            result = cons(b,result);
            V();
            }
        }
    else
        {
        P();
        ENSURE_MEMORY(1,(int *) 0);
        b = POP();
        result = cons(b,0);
        V();
        }

    return result;
    }


/* exprSelect : primary [SYMBOL(.) primary]*
 *            | primary [OBRACKET expr CBRACKET]*
 */

static int
exprSelect(PARSER *p,int item)
    {
    int a,b,c;

    if (TRACE) printf("in exprSelect...\n");

    if (item == 0)
        a = primary(p);
    else
        a = item;

    rethrow(a);

    PUSH(a); /* peek position 0 */

    check(p,0); /* force a lex (not gc-safe) */

    while (opType(p) == SELECT)
        {
        int result;
        //printf("it's a select!\n");
        /* opType forces a lex */
        if (SameSymbol(p->pending,OpenBracketSymbol))
            {
            //printf("it's a bracket!\n");
            b = match(p,SYMBOL);
            PUSH(b);
            c = sexpr(p);
            rethrowPop(c,2); /* for a and b */
            PUSH(c);
            int m = match(p,CLOSE_BRACKET);
            rethrowPop(m,3); /* for a and b and c */
            }
        else /* must be dot (this needs to be modified if more SELECT ops) */
            {
            b = match(p,SYMBOL);
            PUSH(b);
            if (check(p,SYMBOL))
                c = match(p,SYMBOL); /* do not check for undefined here */
            else
                c = primary(p);
            rethrowPop(c,2); /* for a and b */
            PUSH(c); /* to match the true block of this if */
            }
        P();
        ENSURE_MEMORY(3,(int *) 0);
        c = POP();
        b = POP();
        a = PEEK(0);
        result = cons(b,cons(a,cons(c,0)));
        V();
        REPLACE(0,result); /* a gets a new value */
        }
    a = POP();

    if (TRACE) printf("leaving exprSelect.\n");

    return a;
    }

/* primary : INTEGER | REAL | STRING
 *         | SYMBOL
 *         | SYMBOL[return] expr
 *         | QUOTE SYMBOL
 *         | parenExpr
 *         | functionExpr
 *         | block
 */

static int
primary(PARSER *p)
    {
    int r;
    int result;

    if (TRACE) printf("in primary...\n");

    if (check(p,INTEGER))
        result = match(p,INTEGER);
    else if (check(p,REAL))
        result = match(p,REAL);
    else if (check(p,STRING))
        result = match(p,STRING);
    else if (check(p,SYMBOL))
        {
        if (SameSymbol(p->pending,ReturnSymbol))
            {
            /* it's a return */
            match(p,SYMBOL);
            r = sexpr(p);
            rethrow(r);
            P();
            ENSURE_MEMORY(2,&r,(int *) 0);
            result = cons2(ReturnSymbol,cons(r,0));
            V();
            }
        else if (SameSymbol(p->pending,FunctionSymbol))
            {
            r = functionDef(p);
            rethrow(r);
            result = r;
            }
        else
            {
            result = match(p,SYMBOL);
            }
        }
    else if (check(p,QUOTE))
        {
        match(p,QUOTE);
        r = match(p,SYMBOL);
        rethrow(r);
        P();
        ENSURE_MEMORY(2,&r,(int *) 0);
        result = cons2(QuoteSymbol,cons(r,0));
        V();
        }
    else if (check(p,OPEN_PARENTHESIS))
        {
        int m;
        match(p,OPEN_PARENTHESIS);
        r = sexpr(p);
        rethrow(r);
        PUSH(r);
        m = match(p,CLOSE_PARENTHESIS);
        rethrowPop(m,1); /* for r */
        P();
        ENSURE_MEMORY(2,(int *) 0);
        r = POP();
        result = cons2(ParenSymbol,cons(r,0));
        V();
        }
    else if (check(p,OPEN_BRACE))
        {
        r = block(p);
        rethrow(r);
        result = r;
        }
    else if (isThrow(p->pending))
        return p->pending;
    else
        {
        return throw(EmptyExpressionSymbol,
            "file %s,line %d: expected an expression, got %s instead",
            SymbolTable[p->file],p->line,type(p->pending));
        }

    if (TRACE) printf("leaving primary.\n");
    return result;
    }

/* optArgList : argList
 *           | *empty*
 */

static int
optArgList(PARSER *p)
    {
    int result;

    if (TRACE) printf("in optArgList...\n");

    if (check(p,OPEN_BRACE) || isExprPending(p))
        {
        result = argList(p);
        }
    else
        result = 0;

    //debug("arglist is ",result);
    if (TRACE) printf("leaving optArgList.\n");

    return result;
    }

/* argList : expr
 *         | expr COMMA argList
 */

static int
argList(PARSER *p)
    {
    int a,b;
    int result;

    if (TRACE) printf("in argList...\n");
    a = sexpr(p);
    rethrow(a);

    PUSH(a);
    check(p,0); /* force a lex (not gc-safe) */

    if (check(p,COMMA))
        {
        match(p,COMMA);
        b = argList(p);
        rethrowPop(b,1); /* for a */
        }
    else
        {
        b = 0;
        }

    P();
    ENSURE_MEMORY(1,&b,(int *) 0);
    a = POP();
    result = cons(a,b);
    V();

    if (TRACE) printf("leaving argList.\n");
    return result;
    }

/***** pending utilities ************************************************/

static int
isDefinitionPending(PARSER *p)
    {
    check(p,SYMBOL);
    return SameSymbol(p->pending,VarSymbol)
        || SameSymbol(p->pending,FunctionSymbol);
    }

static int
isBlockPending(PARSER *p)
    {
    return check(p, OPEN_BRACE);
    }

static int
isStatementPending(PARSER *p)
    {
    return isExprPending(p);
    }

static int
isExprPending(PARSER *p)
    {
    return check(p,INTEGER) || check(p,REAL) || check(p,STRING)
        || check(p,OPEN_PARENTHESIS) || check(p,SYMBOL)
        || check(p,QUOTE);
    }

static int
opType(PARSER *p)
    {
    //force lex
    check(p,0);

    return operatorType(p->pending);
    }

static int
operatorType(int expr)
    {
    int result;

    //printf("expr address is %d\n", expr);
    //printf("expr type is %s\n", type(expr));
    //printf("expr is %s\n", name(expr));

    if (type(expr) != SYMBOL)
        {
        result = NOTanOP;
        }
    else
        {
        if (SameSymbol(expr,EqSymbol)
        ||  SameSymbol(expr,HeadAssignSymbol)
        ||  SameSymbol(expr,TailAssignSymbol))
            result = UPDATER;
        else if (SameSymbol(expr,AndAndSymbol) || SameSymbol(expr,OrOrSymbol))
            result = LOGICAL_CONNECTIVE;
        else if (SameSymbol(expr,LtSymbol) || SameSymbol(expr,GtSymbol)
        || SameSymbol(expr,EqEqSymbol) ||  SameSymbol(expr,NeqSymbol)
        || SameSymbol(expr,GteSymbol) || SameSymbol(expr,LteSymbol))
            result = LOGICAL_COMPARISON;
        else if (SameSymbol(expr,DotSymbol)
        || SameSymbol(expr,OpenBracketSymbol))
            result = SELECT;
        else
            result = ARITHMETIC;
        }

    return result;
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
        p->pending = swayLex(p);
        //printf("token is %s\n",type(p->pending));
        }

    //printf("type(p->pending) is %s\n",type(p->pending));
    //if (type(p->pending) == SYMBOL) debug("pending is ",p->pending);

    if (isThrow(p->pending))
        return 0;
    else
        return type(p->pending) == t;
    }

static int
isXCall(int expr)
    {
    return type(expr) == CONS && SameSymbol(car(expr),XcallSymbol);
    }

static int
hasDefinitions(int expr)
    {
    while (expr != 0)
        {
        int item = car(expr);
        if (type(item) == CONS)
            {
            if (SameSymbol(car(item),DefineSymbol)) return 1;
            if (SameSymbol(car(item),BeginSymbol)) return 1;
            }
        expr = cdr(expr);
        }
    return 0;
    }
