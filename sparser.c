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

extern void pp(FILE *,int);

#define TRACE 1

/* operator types */

#define UPDATER              0       /* assign           */
#define LOGICAL_CONNECTIVE   1       /* AND, OR, and NOT */
#define LOGICAL_COMPARISON   2       /* <, >, and so on  */
#define ARITHMETIC           3       /* +, -, and so on  */
#define SELECT               4       /* . and [          */
#define NOTanOP              5

extern void debug(char *,int);

/* recursive descent parsing function */

static int optStatementSeq(PARSER *);
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

static int isStatementPending(PARSER *);
static int isBlockPending(PARSER *);
static int isExprPending(PARSER *);
static int opType(PARSER *);
static int isXCall(int);

/* parser support */

static int check(PARSER *,char *);
static int match(PARSER *,char *);
static FILE *openScamFile(char *);
static void ppf(char *,int,char *);

/*
 
//Sway Grammar
//
//   TERMINALS are in (mostly) uppercase
//   nonTerminals are in (mostly) lowercase
//   *empty* is epsilon

//START RULE

    nakedBlock : optDefinitionSeq optStatementSeq
               ;

//DEFINITION RULES

    optDefinitionSeq | *empty*
                     : definition optDefinitionSeq 
                     ;

    definition : functionDef
               | varDef
               | SYMBOL[include] OPAREN expr CPAREN
               | SYMBOL[includeOnce] OPAREN SYMBOL COMMA expr CPAREN
               ;

    //Note: SYMBOL[include] means an identifier whose string name is "include"

    //Note: include function calls are considered definitions 

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

    block : OBRACE optDefinitionSeq optStatementSeq CBRACE

    //Note: blocks with definitions extend the current environment -
    //      a block without definitions is tagged a naked block -
    //      a block that is the body of a function definitions is
    //      tagged a naked block

    optStatementSeq : *empty*
                    | statement optStatementSeq
                    ;

    statement :  block 
              | expr SEMI
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

    printf("parsing now, %s pending\n",type(p->pending));
    result = statement(p);
    rethrow(result,0);
    push(result);
    end = match(p,END_OF_INPUT);
    result = pop();
    rethrow(end,0);

    //assureMemory("parse",1,&result,(int *)0);
    //result = uconsfl(beginSymbol,result,file(result),line(result));
    //printf("done parsing.\n");
    return result;
    }

/*    optStatementSeq : *empty*
 *                   | statement optStatementSeq
 *                   ;
 */

static int
optStatementSeq(PARSER *p)
    {
    int result;

    if (TRACE) printf("in optStatementSeq...\n");

    if (check(p,SYMBOL) && check(p,OPEN_BRACKET))
        {
        assureMemory("expr:throw",1000,(int *)0);
        return throw(syntaxExceptionSymbol,
            "file %s,line %d: expected an expression, got %s instead",
            SymbolTable[p->file],p->line,type(p->pending));
        }

    if (isStatementPending(p))
        {
        int item,others;

        item = statement(p);
        rethrow(item,0);
        //save(item,"parser:optStatementSeq:item");
        push(item);
        others = optStatementSeq(p);
        //item = restore("parser:optStatementSeq:item");
        item = pop();
        rethrow(others,0);
        //result = cons(JOIN,item,others);
        result = ucons(item,others);
        }
    else
        {
        result = 0;
        }

    if (TRACE) ppf("leaving optStatementSeq: ",result,"\n");

    return result;
    }

/*    statement :  block
 *             | expr SEMI
 *             ;
 */

static int
statement(PARSER *p)
    {
    int r,s;
    int m;

    if (TRACE) printf("in statement...\n");

    if (isBlockPending(p))
        {
        assureMemory("expr:throw",1000,(int *)0);
        return throw(syntaxExceptionSymbol,
            "file %s,line %d: expected an expression, got %s instead",
            SymbolTable[p->file],p->line,type(p->pending));
        }

    r = expr(p);

    rethrow(r,0);

    push(r);
    //s = cons(STATEMENT,r,0);
    s = ucons(r,0); //(xcallSymbol,result);
    r = pop();

    if (TRACE > 1) printf("statement subtype is %s\n", type(r));

    line(s) = line(r);
    //indent(s) = indent(r);

    if (!isXCall(r))
        {
        push(s);
        //ppf("statement is: ",r,"\n");
        m = match(p, SEMI);
        s = pop();
        rethrow(m,0);
        }

    if (TRACE) ppf("leaving statement: ",s,"\n");

    return s;
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

    //ppf("exprCall: op was: ",op,"\n");
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
        printf("args is "); pp(stdout,args); printf("\n");
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
        pp(stdout,result);
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
        if (sameSymbol(p->pending,returnSymbol))
            {
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

    printf("arglist is: "); pp(stdout,result); printf("\n");
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
    printf("arg is: "); pp(stdout,a); printf("\n");

    result = uconsfl(a,b,file(a),line(a));

    if (TRACE) printf("leaving argList.\n");
    return result;
    }

/***** pending utilities ************************************************/

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
        p->pending = lex(p);
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

static void
ppf(char *s1,int expr,char *s2)
    {
    fprintf(stdout,"%s",s1);
    pp(stdout,expr);
    fprintf(stdout,"%s",s2);
    }
