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

#define TRACE 0

/* operator types */

#define UPDATER              0       /* assign           */
#define LOGICAL_CONNECTIVE   1       /* AND, OR, and NOT */
#define LOGICAL_COMPARISON   2       /* <, >, and so on  */
#define ARITHMETIC           3       /* +, -, and so on  */
#define SELECT               4       /* . and [          */
#define NOTanOP              5


extern void debug(char *,int);

/* recursive descent parsing function */

static int expr(PARSER *);
static int exprCall(int,PARSER *);
static int exprSelect(int,PARSER *);
static int primary(PARSER *);

/* pending functions */

static int isExprPending(PARSER *);
static int opType(PARSER *);
static int isIdentifier(int,int);

/* parser support */

static int check(PARSER *,char *);
static int match(PARSER *,char *);
static FILE *openScamFile(char *);

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

    argList : arg
            | arg COMMA
            | arg COMMA argList
            ;

    arg : expr
        | SEMI
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

    result = expr(p);
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

/* expr : exprAssign */

static int
expr(PARSER *p)
    {
    return exprCall(0,p);
    }

#ifdef JUNK

/* exprAssign : exprConnect
 *            | exprConnect SYMBOL(=,tail=,head=) exprAssign
 *
 * exprAssign is left associative
 */

static int
exprAssign()
    {
    int a,b,c;
    int result;

    if (TRACE) printf("in exprAssign...\n");

    a = exprConnect();
    rethrow(a,0);

    if (isXCall(a))
        {
        if (TRACE) ppf("leaving exprAssign: ",a,"\n");
        return a;
        }

    save(a,"parser:exprAssign:a");
    if (opType(p) == UPDATER)
        {
        b = match(SYMBOL);
        rethrow(b,1);
        save(b,"parser:exprAssign:b");
        c = exprAssign();
        rethrow(c,2);
        save(c,"parser:exprAssign:c");
        ensureMemory(3);
        c = restore("parser:exprAssign:c");
        b = restore("parser:exprAssign:b");
        a = restore("parser:exprAssign:a"); /* was result */
        result = ucons(JOIN,c,0);
        result = ucons(JOIN,a,result);
        result = ucons(BINARY,b,result);
        line(result) = line(b);
        indent(result) = indent(a);
        }
    else
        result = restore("parser:exprAssign:a"); /* was result */

    if (TRACE) ppf("leaving exprAssign: ",result,"\n");

    return result;
    }

/* exprConnect : exprCompare (SYMBOL(<,<=,==,>=,>,!=) exprConnect)* 
 *
 * exprAssign is left associative
 */

static int
exprConnect()
    {
    int a,b,c;
    int result;

    if (TRACE) printf("in exprConnect...\n");

    a = exprCompare();
    rethrow(a,0);

    if (isXCall(a))
        {
        if (TRACE) ppf("leaving exprConnect: ",a,"\n");
        return a;
        }

    result = a;

    save(result,"parser:exprConnect:result");

    while (opType(p) == LOGICAL_CONNECTIVE)
        {
        b = match(SYMBOL);
        rethrow(b,1);
        save(b,"parser:exprConnect:b");
        c = exprCompare();
        rethrow(c,2);
        save(c,"parser:exprConnect:c");
        ensureMemory(3);
        c = restore("parser:exprConnect:c");
        b = restore("parser:exprConnect:b");
        a = restore("parser:exprConnect:result");
        result = ucons(JOIN,c,0);
        result = ucons(JOIN,a,result);
        result = ucons(BINARY,b,result);
        line(result) = line(a);
        indent(result) = indent(a);
        save(result,"parser:exprConnect:result");
        }

    result = restore("parser:exprConnect:result");

    if (TRACE) ppf("leaving exprConnect: ",result,"\n");

    return result;
    }

/* exprCompare : exprMath (SYMBOL(+,-,*,/,%) exprMath)*
 *
 * exprCompare is left associative
 */

static int
exprCompare()
    {
    int a,b,c;
    int result;

    if (TRACE) printf("in exprCompare...\n");

    a = exprMath();
    rethrow(a,0);

    if (isXCall(a))
        {
        if (TRACE) ppf("leaving exprCompare: ",a,"\n");
        return a;
        }

    result = a;
    save(result,"parser:exprCompare:result");

    while (opType(p) == LOGICAL_COMPARISON)
        {
        b = match(SYMBOL);
        rethrow(b,1);
        save(b,"parser:exprCompare:b");
        c = exprMath();
        rethrow(c,2);
        save(c,"parser:exprCompare:c");
        ensureMemory(3);
        c = restore("parser:exprCompare:c");
        b = restore("parser:exprCompare:b");
        a = restore("parser:exprCompare:result");
        result = ucons(JOIN,c,0);
        result = ucons(JOIN,a,result);
        result = ucons(BINARY,b,result);
        line(result) = line(a);
        indent(result) = indent(a);
        save(result,"parser:exprCompare:result");
        }

    result = restore("parser:exprCompare:result");

    if (TRACE) ppf("leaving exprCompare: ",result,"\n");

    return result;
    }

/* exprMath : exprCall (SYMBOL exprCall)* */

static int
exprMath()
    {
    int a,b,c;
    int result;

    if (TRACE) printf("in exprMath...\n");

    a = exprCall(0);
    rethrow(a,0);

    if (isXCall(a))
        {
        if (TRACE) ppf("leaving exprMath: ",a,"\n");
        return a;
        }

    result = a;
    //ppf("exprMath: a was: ",a,"\n");

    save(result,"parser:exprMath:result");

    while (opType(p) == ARITHMETIC)
        {
        b = match(SYMBOL);
        //ppf("exprMath: b was: ",b,"\n");
        rethrow(b,1);
        save(b,"parser:exprMath:b");
        c = exprCall(0);
        //ppf("exprMath: c was: ",c,"\n");
        rethrow(c,2);
        save(c,"parser:exprMath:c");
        ensureMemory(3);
        c = restore("parser:exprMath:c");
        b = restore("parser:exprMath:b");
        a = restore("parser:exprMath:result");
        //ppf("exprMath: a is: ",a,"\n");
        //ppf("exprMath: b is: ",b,"\n");
        //ppf("exprMath: c is: ",c,"\n");
        result = ucons(JOIN,c,0);
        result = ucons(JOIN,a,result);
        result = ucons(BINARY,b,result);
        line(result) = line(a);
        indent(result) = indent(a);
        //ppf("exprMath: a was: ",result,"\n");
        save(result,"parser:exprMath:result");
        }

    result = restore("parser:exprMath:result");

    if (TRACE) ppf("leaving exprMath: ",result,"\n");

    return result;
    }
#endif

/* exprCall : exprSelect
 *          | exprSelect OPAREN optArgList CPAREN optExtraArgs
 *          | exprSelect OPAREN optArgList CPAREN optExtraArgs
 *              SYMBOL(.) exprCall
 */

static int
exprCall(int item,PARSER *p)
    {
    int op,args,extra;
    int result;
    int m;

    if (TRACE) printf("in exprCall...\n");

    if (item == 0)
        op = exprSelect(0,p);
    else
        op = item;

    //ppf("exprCall: op was: ",op,"\n");
    rethrow(op,0);

    push(op);

    if (check(p,OPEN_PARENTHESIS)) /* check is not gc-safe */
        {
        m = match(p,OPEN_PARENTHESIS);
        rethrow(args = optArgList(p),1);
        push(args);
        rethrow(m = match(p,CLOSE_PARENTHESIS),2);
        //modify the next line when extra args is implemented
        rethrow(extra = 0); //optExtraArgs(p),2);
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
        if (extra != 0)
            {
            result = ucons(xcallSymbol,result);
            }
        if (opType(p) == SELECT)
            {
            result = exprCall(exprSelect(result,p));
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
exprSelect(int item,PARSER *p)
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
        if (isIdentifier(p->pending,openBracketSymbol))
            {
            int closer;
            rethrow(b = match(p,SYMBOL),1);
            push(b);
            rethrow(c = expr(p),2);
            push(c);
            closer = match(p,SYMBOL);
            if (closer != closeBracketSymbol)
                return throw(syntaxExceptionSymbol,
                    "file %s,line %d: expected a close bracket, got %s instead",
                    SymbolTable[p->file],p->line,
                    SymbolTable[ival(closer)]);
            }
        else /* must be dot */
            {
            rethrow(b = match(p,SYMBOL),1);
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

    if (check(p,INTEGER))
        result = match(p,INTEGER);
    else if (check(p,REAL))
        result = match(p,REAL);
    else if (check(p,STRING))
        result = match(p,STRING);
    else if (check(p,SYMBOL))
        {
        if (isIdentifier(p->pending,returnSymbol))
            {
            rethrow(r = expr(p),0);
            assureMemory("primary:return",2,&r,(int *)0);
            result = uconsfl(returnSymbol,ucons(r,0),file(r),line(r));
            }
        else if (isIdentifier(p->pending,functionSymbol))
            {
            fprintf(stderr,"lambdas not yet implemented\n");
            exit(-1);
            }
        else
            result = match(p,SYMBOL);
        }
    else if (check(p,QUOTE))
        {
        match(p,QUOTE);
        rethrow(r = match(p,SYMBOL),0);
        assureMemory("primary:quote",2,&r,(int *)0);
        result = uconsfl(quoteSymbol,ucons(r,0),file(r),line(r));
        }
    else if (check(p,OPEN_PARENTHESIS))
        {
        r = expr(p);
        rethrow(r,0);
        rethrow(match(p,CLOSE_PARENTHESIS),0);
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

    return result;
    }

/* optArgList : argList
 *           | *empty*
 */

static int
optArgList()
    {
    if (check(OPEN_BRACE) || isExprPending())
        return argList();
    else
        return 0;
    }

/* argList : arg
 *         | arg COMMA argList
 */

static int
argList()
    {
    int a,b;
    int result;

    a = arg();
    rethrow(a,0);
    save(a,"parser:argList:a");

    if (check(COMMA)) /* check is not gc-safe */
        {
        match(COMMA);
        if (check(CLOSE_PARENTHESIS))
            b = 0;
        else
            b = argList();
        a = restore("parser:argList:a");
        rethrow(b,0);
        }
    else
        {
        b = 0;
        a = restore("parser:argList:a");
        }

    result = cons(JOIN,a,b);

    return result;
    }

/***** pending utilities ************************************************/

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
    int Pending = p->pending;

    //force lex
    check(p,0);

    if (type(Pending) != SYMBOL)
        {
        result = NOTanOP;
        }
    else
        {
        if (isIdentifier(Pending,assignSymbol)
        ||  isIdentifier(Pending,headAssignSymbol)
        ||  isIdentifier(Pending,tailAssignSymbol))
            result = UPDATER;
        else if (isIdentifier(Pending,andAndSymbol) || isIdentifier(Pending,orOrSymbol))
            result = LOGICAL_CONNECTIVE;
        else if (isIdentifier(Pending,ltSymbol) || isIdentifier(Pending,gtSymbol)
        || isIdentifier(Pending,eqSymbol) ||  isIdentifier(Pending,neqSymbol)
        || isIdentifier(Pending,gteSymbol) || isIdentifier(Pending,lteSymbol))
            result = LOGICAL_COMPARISON;
        else if (isIdentifier(Pending,dotSymbol)
        || isIdentifier(Pending,openBracketSymbol))
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

    //printf("type(p->pending) is %s\n",type(p->pending)); //if (type(p->pending) == SYMBOL) ppf("pending is ",p->pending,"\n");

    if (isThrow(p->pending))
        return 0;
    else
        return type(p->pending) == t;
    }

static int
isIdentifier(int token,int sym)
    {
    return type(token) == SYMBOL && ival(token) == ival(sym);
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
