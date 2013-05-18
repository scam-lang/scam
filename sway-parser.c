# include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <assert.h>
#include "cell.h"
#include "types.h"
#include "parser.h"
#include "env.h"
#include "util.h"

extern void swayPP(FILE *,int);
extern int swayLex(PARSER *);

#define TRACE 2

/* operator types */

#define UPDATER              0       /* assign           */
#define LOGICAL_CONNECTIVE   1       /* AND, OR, and NOT */
#define LOGICAL_COMPARISON   2       /* <, >, and so on  */
#define ARITHMETIC           3       /* +, -, and so on  */
#define SELECT               4       /* . and [          */
#define NOTanOP              5

extern void debug(char *,int);

/* recursive descent parsing function */

static int functionDef(PARSER *);
static int varDef(PARSER *);
static int varDefList(PARSER *);
static int varDefItem(PARSER *);
static int optParamList(PARSER *);
static int paramList(PARSER *);
static int nakedBlock(PARSER *);
static int statement(PARSER *);
static int expr(PARSER *);
static int exprAssign(PARSER *);
static int exprConnect(PARSER *);
static int exprCompare(PARSER *);
static int exprMath(PARSER *);
static int exprCall(PARSER *,int);
static int exprSelect(PARSER *,int);
static int primary(PARSER *);
static int optArgList(PARSER *);
static int argList(PARSER *);

/* pending functions */

static int isDefinitionPending(PARSER *p);
static int isStatementPending(PARSER *);
static int isBlockPending(PARSER *);
static int isExprPending(PARSER *);
static int opType(PARSER *);
static int isXCall(int);

/* parser support */

static int check(PARSER *,char *);
static int match(PARSER *,char *);
static void ppf(char *,int,char *);

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

    printf("parsing now, %s pending\n",type(p->pending));
    result = nakedBlock(p);
    rethrow(result,0);
    push(result);
    end = match(p,END_OF_INPUT);
    result = pop();
    rethrow(end,0);

    assureMemory("swayParse",1,&result,(int *)0);
    result = uconsfl(beginSymbol,result,file(result),line(result));
    printf("done parsing.\n");
    ppf("sway parser returns\n",result,"\n");
    return result;
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

    if (check(p,SYMBOL) && sameSymbol(p->pending,varSymbol))
        {
        d = varDef(p);
        rethrow(d,0);
        push(d);
        m = match(p,SEMI);
        d = pop();
        rethrow(m,0);
        }
    else if (check(p,SYMBOL) && sameSymbol(p->pending,functionSymbol))
        {
        d = functionDef(p);
        rethrow(d,0);
        }
    else /* must be call to include or includeOnce */
        {
        d = expr(p);
        rethrow(d,0);
        if (!sameSymbol(car(d),xcallSymbol))
            {
            push(d);
            m = match(p,SEMI);
            d = pop();
            rethrow(m,0);
            }
        }

    if (TRACE) ppf("leaving definition: ",d,"\n");

    return d;
    }

/*
 *  functionDef : FUNCTION OPAREN optParamList block
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
    rethrow(f,0);

    assert(sameSymbol(f,functionSymbol));

    if (check(p,SYMBOL))
        v = match(p,SYMBOL);
    else
        {
        v = newSymbol("anonymous");
        anonymous = 1;
        }

    push(v);

    m = match(p,OPEN_PARENTHESIS);
    rethrow(m,1);

    params = optParamList(p);
    rethrow(params,1);

    push(params);

    m = match(p,CLOSE_PARENTHESIS);
    rethrow(m,2);

    m = match(p,OPEN_BRACE);
    rethrow(m,2);

    printf("about to parse function body\n");
    getchar();

    body = nakedBlock(p);

    rethrow(body,2);

    push(body);

    m = match(p,CLOSE_BRACE);
    rethrow(m,3);

    if (anonymous)
        {
        m = match(p,SEMI);
        rethrow(m,3);
        }

    //ensureMemory(6);
    assureMemory("varDefItem",6,(int *)0);

    body = pop();
    params = pop();
    v = pop();

    if (car(body) == 0)
        {
        return throw(syntaxExceptionSymbol,
            "file %s, line %d: blocks must contain at least one statement",
            SymbolTable[p->file], p->line, type(p->pending));
        }

    /*
    result = ucons(JOIN,v,0);
    result = ucons(JOIN,body,result);
    result = ucons(FUNCTION,params,result);
    result = ucons(FUNCTION_DEFINITION,v,result);
    */

    //indent the name so that we can find the indent after the call

    assureMemory("func name",4,&body,&params,&v,(int *)0);
    result = ucons(v,params);
    result = ucons(result,body);
    result = uconsfl(defineSymbol,result,file(v),line(v));

    if (TRACE) ppf("leaving functionDef: ",result,"\n");

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
    rethrow(m,0);
    //assert(isIdentifier(m,"var"));
    assert(sameSymbol(m,varSymbol));
    v = varDefList(p);
    rethrow(v,0);
    //type(v) = VARIABLE_DEFINITION_LIST;

    if (TRACE) ppf("leaving varDef: ",v,"\n");

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
    rethrow(item,0);
    //save(item,"parser:varDefList:item");
    push(item);

    if (check(p,COMMA)) /* check is not gc-safe */
        {
        match(p,COMMA);
        result = varDefList(p);
        //item = restore("parser:varDefList:item");
        item = pop();
        rethrow(result,0);
        }
    else
        {
        item = pop();
        result = 0;
        }

    //result = cons(JOIN,item,result);
    result = ucons(item, result);

    if (TRACE) ppf("leaving varDefList: ",result,"\n");

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
    rethrow(v,0);
    push(v);

    check(p,SYMBOL); // force a lex, if necessary (check is not gc-safe)

    //if (isIdentifier(Pending,ASSIGN))
    if (sameSymbol(p->pending,eqSymbol))
        {
        match(p,SYMBOL); // ASSIGN
        init = expr(p);
        }
    else
        {
        if (TRACE > 1) printf("no initializer\n");
        init = newSymbol(UNINITIALIZED);
        //init = newSymbol(uninitializedVariable);
        }

    rethrow(init,1); //for saved v

    push(init);

    //ensureMemory(4);
    assureMemory("varDefItem",4,(int *)0);

    init = pop();
    v = pop();

    result = ucons(init,0);
    result = ucons(v,result);
    result = ucons(newSymbol("var"),result);

    if (TRACE) ppf("leaving varDefItem: ",result,"\n");

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
    rethrow(a,0);

    push(a);

    if (check(p,COMMA)) /* check is not gc-safe */
        {
        match(p,COMMA);
        if (check(p,SYMBOL))
            b = paramList(p);
        else
            b = 0;
        a = pop();
        rethrow(b,0);
        }
    else
        {
        a = pop();
        b = 0;
        }

    result = ucons(a,b);

    if (TRACE) ppf("leaving paramList: ",result,"`\n");

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
        assureMemory("expr:throw",1000,(int *)0);
        return throw(syntaxExceptionSymbol,
            "file %s,line %d: expected an expression, got %s instead",
            SymbolTable[p->file],p->line,type(p->pending));
        }

    ppf("pending: ",p->pending,"\n");
    if (isDefinitionPending(p))
        {
        item = definition(p);
        rethrow(item,0);
        push(item);
        others = nakedBlock(p);
        item = pop();
        rethrow(others,0);
        assureMemory("definition",1,&item,&others,(int *)0);
        result = ucons(item,others);
        }
    else if (isStatementPending(p))
        {
        item = statement(p);
        rethrow(item,0);
        push(item);
        others = nakedBlock(p);
        item = pop();
        rethrow(others,0);
        assureMemory("statement",1,&item,&others,(int *)0);
        result = ucons(item,others);
        }
    else
        {
        result = 0;
        }

    if (TRACE) ppf("leaving nakedBlock: ",result,"\n");

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
    getchar();

    if (isBlockPending(p))
        {
        assureMemory("expr:throw",1000,(int *)0);
        return throw(syntaxExceptionSymbol,
            "file %s,line %d: expected an expression, got a block instead",
            SymbolTable[p->file],p->line);
        }

    r = expr(p);
    printf("statement got expression \n");
    rethrow(r,0);
    printf("statement no error\n");

    if (TRACE > 1) printf("statement subtype is %s\n", type(r));

    if (!isXCall(r))
        {
        ppf("statement is: ",r,"\n");
        ppf("pending is: ", p->pending, "\n");
        m = match(p, SEMI);
        printf("statement match semi\n");
        rethrow(m,0);
        }

    if (TRACE) ppf("leaving statement: ",r,"\n");

    return r;
    }

/* expr : exprAssign */

static int
expr(PARSER *p)
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
    int result;

    if (TRACE) printf("in exprAssign...\n");

    a = exprConnect(p);
    rethrow(a,0);

    if (isXCall(a))
        {
        if (TRACE) ppf("leaving exprAssign: ",a,"\n");
        return a;
        }

    push(a);

    if (opType(p) == UPDATER)
        {
        b = match(p,SYMBOL);
        rethrow(b,1);
        push(b);
        c = exprAssign(p);
        rethrow(c,2);
        push(c);
        assureMemory("exprAssign",3,(int *)0);
        c = pop();
        b = pop();
        a = pop();
        result = uconsfl(b,ucons(a,ucons(c,0)),file(b),line(b));
        }
    else
        result = pop();

    if (TRACE) ppf("leaving exprAssign: ",result,"\n");

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
    int result;

    if (TRACE) printf("in exprConnect...\n");

    a = exprCompare(p);
    rethrow(a,0);

    if (isXCall(a))
        {
        if (TRACE) ppf("leaving exprConnect: ",a,"\n");
        return a;
        }

    result = a;

    push(result);

    while (opType(p) == LOGICAL_CONNECTIVE)
        {
        b = match(p,SYMBOL);
        rethrow(b,1);
        push(b);
        c = exprCompare(p);
        rethrow(c,2);
        push(c);
        assureMemory("exprConnect",3,(int *)0);
        c = pop();
        b = pop();
        a = pop();
        result = uconsfl(b,ucons(a,ucons(c,0)),file(b),line(b));
        push(result);
        }

    result = pop();

    if (TRACE) ppf("leaving exprConnect: ",result,"\n");

    return result;
    }

/* exprCompare : exprMath (SYMBOL[<,<=,==,!=,>=,>] exprMath)*
 *
 * exprCompare is left associative
 */

static int
exprCompare(PARSER *p)
    {
    int a,b,c;
    int result;

    if (TRACE) printf("in exprCompare...\n");

    a = exprMath(p);
    rethrow(a,0);

    if (isXCall(a))
        {
        if (TRACE) ppf("leaving exprCompare: ",a,"\n");
        return a;
        }

    result = a;
    push(result);

    while (opType(p) == LOGICAL_COMPARISON)
        {
        b = match(p,SYMBOL);
        rethrow(b,1);
        push(b);
        printf("about to call math\n");
        c = exprMath(p);
        rethrow(c,2);
        push(c);
        assureMemory("exprCompare",3,(int *)0);
        c = pop();
        b = pop();
        a = pop();
        result = uconsfl(b,ucons(a,ucons(c,0)),file(b),line(b));
        push(result);
        }

    result = pop();

    if (TRACE) ppf("leaving exprCompare: ",result,"\n");

    return result;
    }

/* exprMath : exprCall (SYMBOL[+,-,*,/,%,^] exprCall)* */

static int
exprMath(PARSER *p)
    {
    int a,b,c;
    int result;

    if (TRACE) printf("in exprMath...\n");

    a = exprCall(p,0);
    rethrow(a,0);

    if (isXCall(a))
        {
        if (TRACE) ppf("leaving exprMath: ",a,"\n");
        return a;
        }

    result = a;
    //ppf("exprMath: a was: ",a,"\n");

    push(result);

    while (opType(p) == ARITHMETIC)
        {
        b = match(p,SYMBOL);
        //ppf("exprMath: b was: ",b,"\n");
        rethrow(b,1);
        push(b);
        c = exprCall(p,0);
        //ppf("exprMath: c was: ",c,"\n");
        rethrow(c,2);
        push(c);
        assureMemory("exprMath",3,(int *)0);
        c = pop();
        b = pop();
        a = pop();
        //ppf("exprMath: a is: ",a,"\n");
        //ppf("exprMath: b is: ",b,"\n");
        //ppf("exprMath: c is: ",c,"\n");
        result = uconsfl(b,ucons(a,ucons(c,0)),file(b),line(b));
        //ppf("exprMath: a was: ",result,"\n");
        push(result);
        }

    result = pop();

    if (TRACE) ppf("leaving exprMath: ",result,"\n");

    return result;
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
        op = exprSelect(p,0);
    else
        op = item;

    ppf("exprCall: op was: ",op,"\n");
    getchar();
    rethrow(op,0);

    push(op);

    if (check(p,OPEN_PARENTHESIS)) /* check is not gc-safe */
        {
        //this is a function call
        match(p,OPEN_PARENTHESIS);
        printf("about to get args\n");
        args = optArgList(p);
        rethrow(args,1);
        printf("done getting args\n");
        ppf("args is ",args,"\n");
        push(args);
        m = match(p,CLOSE_PARENTHESIS);
        rethrow(m,2);
        //modify the next line when extra args is implemented
        extra = 0; //optExtraArgs(p),2);
        rethrow(extra,2);
        push(extra);
        //ppf("exprCall: extra was :",extra,"\n");
        assureMemory("exprCall",2,(int *)0);
        extra = pop();
        args = pop();
        op = pop();
        //ppf("exprCall: extra is :",extra,"\n");
        //ppf("exprCall: args is :",args,"\n");
        //ppf("exprCall: op is :",op,"\n");
        /* append is destructive, does not use cons */
        result = uconsfl(op,append(args,extra),file(op),line(op));
        ppf("result is ",result,"\n");
        if (extra != 0)
            {
            result = ucons(xcallSymbol,result);
            }
        if (opType(p) == SELECT)
            {
            result = exprCall(p,exprSelect(p,result));
            }
        }
    else
        {
        result = pop();
        }

    if (TRACE) printf("leaving exprCall.\n");

    return result;
    }

/* exprSelect : primary [SYMBOL(.) primary]*
 *            | primary [OBRACKET expr CBRACKET]*
 */

static int
exprSelect(PARSER *p,int item)
    {
    int a,b,c;
    int result;

    if (TRACE) printf("in exprSelect...\n");

    if (item == 0)
        a = primary(p);
    else
        a = item;

    rethrow(a,0);

    result = a;
    push(result);

    while (opType(p) == SELECT)
        {
        /* opType forces a lex */
        if (check(p,OPEN_BRACKET))
            {
            match(p,OPEN_BRACKET);
            c = expr(p);
            rethrow(c,1);
            push(c);
            match(p,CLOSE_BRACKET);
            b = openBracketSymbol;
            }
        else /* must be dot */
            {
            b = match(p,SYMBOL);
            rethrow(b,1);
            push(b);
            if (check(p,SYMBOL))
                c = match(p,SYMBOL); //do not check for undefined here
            else
                c = primary(p);
            rethrow(c,2);
            push(c);
            }
        assureMemory("exprSelect",3,(int *)0);
        c = pop();
        b = pop();
        a = pop();
        result = uconsfl(b,ucons(a,ucons(c,0)),file(b),line(b));
        push(result);
        }

    result = pop();

    if (TRACE) printf("leaving exprSelect.\n");

    return result;
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
    //printf("primary: %s pending\n",type(p->pending));
    //getchar();

    if (check(p,INTEGER))
        result = match(p,INTEGER);
    else if (check(p,REAL))
        result = match(p,REAL);
    else if (check(p,STRING))
        result = match(p,STRING);
    else if (check(p,SYMBOL))
        {
        printf("primary symbol\n");
        getchar();
        if (sameSymbol(p->pending,returnSymbol))
            {
            printf("it's a return\n");
            getchar();
            match(p,SYMBOL);
            r = expr(p);
            rethrow(r,0);
            assureMemory("primary:return",2,&r,(int *)0);
            result = uconsfl(returnSymbol,ucons(r,0),file(r),line(r));
            }
        else if (sameSymbol(p->pending,functionSymbol))
            {
            fprintf(stderr,"lambdas not yet implemented\n");
            exit(-1);
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
        rethrow(r,0);
        assureMemory("primary:quote",2,&r,(int *)0);
        result = uconsfl(quoteSymbol,ucons(r,0),file(r),line(r));
        }
    else if (check(p,OPEN_PARENTHESIS))
        {
        int m;
        match(p,OPEN_PARENTHESIS);
        r = expr(p);
        rethrow(r,0);
        m = match(p,CLOSE_PARENTHESIS);
        rethrow(m,0);
        assureMemory("expr:openParen",2,&r,(int *)0);
        result = uconsfl(parenSymbol,ucons(r,0),file(r),line(r));
        }
    else if (check(p,OPEN_BRACE))
        {
        fprintf(stderr,"blocks not yet implemented\n");
        exit(-1);
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

    ppf("arglist is ",result,"\n");
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
    a = expr(p);
    rethrow(a,0);
    push(a);

    //force check
    check(p,0);
    if (sameSymbol(p->pending,commaSymbol))
        {
        match(p,SYMBOL);
        if (check(p,CLOSE_PARENTHESIS))
            b = 0;
        else
            b = argList(p);
        rethrow(b,1);
        }
    else
        {
        b = 0;
        }

    assureMemory("argList",1,&b,(int *)0);

    a = pop();
    ppf("arg is ",a,"\n");

    result = uconsfl(a,b,file(a),line(a));

    if (TRACE) printf("leaving argList.\n");
    return result;
    }

/***** pending utilities ************************************************/

static int
isDefinitionPending(PARSER *p)
    {
    check(p,SYMBOL);
    return sameSymbol(p->pending,varSymbol)
        || sameSymbol(p->pending,functionSymbol);
    }

static int
isBlockPending(PARSER *p)
    {
    return check(p, OPEN_BRACE);
    }

static int
isStatementPending(PARSER *p)
    {
    return isExprPending(p) || isBlockPending(p);
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
    int result;
    int Pending;

    //force lex
    check(p,0);

    Pending = p->pending;

    //printf("Pending address is %d\n", Pending);
    //printf("Pending type is %s\n", type(Pending));
    //printf("Pending is %s\n", name(Pending));

    if (type(Pending) != SYMBOL)
        {
        result = NOTanOP;
        }
    else
        {
        if (sameSymbol(Pending,eqSymbol)
        ||  sameSymbol(Pending,headAssignSymbol)
        ||  sameSymbol(Pending,tailAssignSymbol))
            result = UPDATER;
        else if (sameSymbol(Pending,andAndSymbol) || sameSymbol(Pending,orOrSymbol))
            result = LOGICAL_CONNECTIVE;
        else if (sameSymbol(Pending,ltSymbol) || sameSymbol(Pending,gtSymbol)
        || sameSymbol(Pending,eqEqSymbol) ||  sameSymbol(Pending,neqSymbol)
        || sameSymbol(Pending,gteSymbol) || sameSymbol(Pending,lteSymbol))
            result = LOGICAL_COMPARISON;
        else if (sameSymbol(Pending,dotSymbol)
        || sameSymbol(Pending,openBracketSymbol))
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
        p->pending = swayLex(p);
        //printf("token is %s\n",type(p->pending));
        }

    //printf("type(p->pending) is %s\n",type(p->pending));
    //if (type(p->pending) == SYMBOL) ppf("pending is ",p->pending,"\n");

    if (isThrow(p->pending))
        return 0;
    else
        return type(p->pending) == t;
    }

static int
isXCall(int expr)
    {
    return type(expr) == CONS && sameSymbol(car(expr),xcallSymbol);
    }

static void
ppf(char *s1,int expr,char *s2)
    {
    extern void pp(FILE *,int);
    fprintf(stdout,"%s",s1);
    pp(stdout,expr);
    fprintf(stdout,"%s",s2);
    }
