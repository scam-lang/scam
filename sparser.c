#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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

FILE *Input;
FILE *Output;
char *InputString;
int InputStringLength;
int InputStringIndex;

/* the pending input token, -1 is not assigned */

int Pending = -1;

/* recursive descent parsing function */

static int nakedBlock(void);
static int varDef(void);
static int varDefList(void);
static int varDefItem(void);
static int functionDef(PARSER *);
static int statement(void);
static int optStatementSeq(void);
static int definition(void);
static int optDefinitionSeq(void);
static int expr(void);
static int exprAssign(void);
static int exprConnect(void);
static int exprCompare(void);
static int exprMath(void);
static int exprCall(int);
static int exprSelect(int);
static int optArgList(void);
static int argList(void);
static int arg(void);
static int optExtraArgs(void);
static int extraArgs(void);
static int optParamList(void);
static int paramList(void);
static int idExpr(void);
static int returnExpr(int);
static int functionExpr(int);
static int parenExpr(void);
static int block(void);
static int primary(void);

/* pending functions */

static int opType(void);
static int isStatementPending(void);
static int isDefinitionPending(void);
static int isExprPending(void);
static int isXCall(int);
static int isBlockPending(void);

/* parser support */

static int check(PARSER *,char *);
static int match(PARSER *,char *);
static FILE *openScamFile(char *);

static char *lastSavedString = 0;

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

/*
 
//Sway Grammar
//
//   TERMINALS are in (mostly) uppercase
//   nonTerminals are in (mostly) lowercase
//   *empty* is epsilon

//START RULE

    nakedBlock : optDefinitionSeq optStatementSeq
               ;

//ALTERNATE START RULE (for interactive use)

    interactive : definition
                | statement
                ;

//DEFINITION RULES

    optDefinitionSeq | *empty*
                     : definition optDefinitionSeq 
                     ;

    definition : functionDef
               | varDef
               | ID[include] OPAREN expr CPAREN
               | ID[includeOnce] OPAREN SYMBOL COMMA expr CPAREN
               ;

    //Note: ID[include] means an identifier whose string name is "include"

    //Note: include function calls are considered definitions 

    functionDef : FUNCTION OPAREN optParamList block
                ;

    varDef : VAR varDefList
           ;
    
    varDefList : varDefItem
               | varDefItem COMMA varDefList
               ;
           
    varDefItem : ID 
               | VAR ID ID[=] expr
               ;

    optParamList : *empty*
                 | paramList
                 ;

    paramList : ID
              | ID COMMA 
              | ID COMMA paramList
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
               | exprConnect ID[=,tail=,head=] exprAssign
               ;
    
    //Note: exprAssign is left associative, the others right associative

    exprConnect : exprCompare [ID[&&,||] exprConnect]*
                ;

    exprCompare : exprMath [ID[<,<=,==,>=,>,!=] exprMath]*
                ;

    exprMath : exprCall (ID exprCall)*
             ;

    exprCall : exprSelect
             | exprSelect OPAREN optArgList CPAREN optExtraArgs
             | exprSelect OPAREN optArgList CPAREN optExtraArgs ID[.] exprCall
             ;

    //Note: a block that immediately follows a call is considered another
    //      argument to the call; optExtraArgs matches these blocks

    exprSelect : primary (ID[.] primary)*
               | primary (OBRACKET expr CBRACKET)*
               ;

    primary : INTEGER
            | REAL
            | STRING
            | SYMBOL
            | ID
            | ID[return] expr
            | functionExpr
            | parenExpr
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

/* this is the function that interaces the parser with other modules */

int
parse(PARSER *p)
    {
    int result,end;

    //printf("starting to parse...\n");
    if (check(p,END_OF_INPUT)) return 0;

    result = functionDef(p);
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

/* nakedBlock : optDefinitionSeq optStatementSeq
 *
 * this is the start rules for non-interactive sessions
 *
 */

static int
nakedBlock()
    {
    int a,b,result;

    if (TRACE) printf("in nakedBlock...\n");

    a = optDefinitionSeq();

    rethrow(a,0);

    push(a,"parser.c:nakedBlock:a");
    b = optStatementSeq();
    a = restore("parser.c:nakedBlock:a");

    rethrow(b,0);

    result = append(a,b);  /* append is gc-safe */

    result = cons(NAKED_BLOCK,result,0);

    if (TRACE) ppf("leaving nakedBlock: ",result,"\n");

    return result;
    }

/* optDefinitionSeq : *empty*
 *                  | definitionOptDefinitionSeq
 */

static int
optDefinitionSeq()
    {
    int d,defs;
    int result;

    if (TRACE) printf("in optDefinitionSeq...\n");

    if (isDefinitionPending())
        {
        d = definition();
        rethrow(d,0);
        push(d,"optDefinitionSeq:d");
        defs = optDefinitionSeq();
        d = restore("optDefinitionSeq:d");
        rethrow(defs,0);
        result = cons(JOIN,d,defs);
        if (TRACE > 1) ppf("definitions now are: ",result,"\n");
        }
    else
        {
        if (TRACE > 1) printf("no definitions present\n");
            result = 0;
        }

    if (TRACE) ppf("leaving optDefinitionSeq: ",result,"\n");

    return result;
    }

/* interactive : definition
 *             | statement
 */

/* parse errors throw Sway exceptions */

int
interactive()
    {
    int d;
    int m;

    if (TRACE) printf("interactive...\n");

    check(0); /* force lex */

    if (isDefinitionPending())
        {
        d = definition();
        rethrow(d,0);
        if (type(d) == XCALL)
            {
            push(d,"parser:interactive:d");
            m = match(SEMI);
            d = restore("parser:interactive:d");
            rethrow(m,0);
            }
        }
    else if (isStatementPending())
        {
        d = statement();
        rethrow(d,0);
        if (type(car(d)) == XCALL)
            {
            push(d,"parser:interactive:d");
            m = match(SEMI);
            d = restore("parser:interactive:d");
            rethrow(m,0);
            }
        }
    else
        {
        return match(END_OF_INPUT);
        }

    /* turn d into naked block */

    push(d,"parser:interactive:d");
    ensureMemory(2);
    d = restore("parser:interactive:d");
    
    d = ucons(NAKED_BLOCK,ucons(JOIN,d,0),0);
    if (TRACE) printf("leaving interactive\n");

    return d;
    }

/* definition : functionDef | varDef | include/includeOnce call */

static int
definition()
    {
    int d;
    int m;
   
    if (TRACE) printf("in definition...\n");

    if (check(ID) && isIdentifier(Pending,"var"))
        {
        d = varDef();
        rethrow(d,0);
        push(d,"parser:definition:d");
        m = match(SEMI);
        d = restore("parser:definition:d");
        rethrow(m,0);
        }
    else if (check(ID) && isIdentifier(Pending,"function"))
        {
        d = functionDef();
        rethrow(d,0);
        }
    else /* must be call to include or includeOnce */
        {
        d = expr();
        rethrow(d,0);
        if (type(d) != XCALL)
            {
            push(d,"parser:definition:d");
            m = match(SEMI);
            d = restore("parser:definition:d");
            rethrow(m,0);
            }
        }

    if (TRACE) ppf("leaving definition: ",d,"\n");

    return d;
    }

/* varDef : VAR varDefList
 *  
 * varDefList : varDefItem
 *            | varDefItem COMMA varDefList
 *   
 * varDefItem : ID 
 *            | ID ID(=) expr
 */

static int
varDef()
    {
    int v;
    int m;
    int indentation;

    if (TRACE) printf("in varDef...\n");

    m = match(ID);
    indentation = indent(m);
    rethrow(m,0);
    assert(isIdentifier(m,"var"));
    v = varDefList();
    rethrow(v,0);
    type(v) = VARIABLE_DEFINITION_LIST;
    line(v) = line(m);
    indent(v) = indentation;

    if (TRACE) ppf("leaving varDef: ",v,"\n");

    return v;
    }

static int
varDefList()
    {
    int item,result;

    if (TRACE) printf("in varDefList...\n");

    item = varDefItem();
    rethrow(item,0);
    push(item,"parser:varDefList:item");

    if (check(COMMA)) /* check is not gc-safe */
        {
        match(COMMA);
        result = varDefList();
        item = restore("parser:varDefList:item");
        rethrow(result,0);
        }
    else
        {
        item = restore("parser:varDefList:item");
        result = 0;
        }

    result = cons(JOIN,item,result);

    if (TRACE) ppf("leaving varDefList: ",result,"\n");

    return result;
    }
    
static int
varDefItem()
    {
    int v,init,result;

    if (TRACE) printf("in varDefItem...\n");

    v = match(ID);
    rethrow(v,0);
    push(v,"parser:varDefItem:v");

    check(ID); /* force a lex, if necessary (check is not gc-safe) */

    if (isIdentifier(Pending,ASSIGN))
        {
        match(ID); /* ASSIGN */
        init = expr();
        }
    else
        {
        if (TRACE > 1) printf("no initializer\n");
        init = newSymbol(UNINITIALIZED);
        }

    rethrow(init,1); //for push v

    push(init,"parser:varDefItem:init");

    ensureMemory(4);

    init = restore("parser:varDefItem:init");
    v = restore("parser:varDefItem:v");

    result = ucons(JOIN,init,0);
    result = ucons(JOIN,v,result);
    result = cons(CALL,newIdentifier("var"),result);
    
    if (TRACE) ppf("leaving varDefItem: ",result,"\n");

    return result;
    }

/* functionDef : FUNCTION ID OPAREN optParamList
 *                   CBRACE nakedBlock CBRACE
 */

static int
functionDef(PARSER *p)
    {
    int f,v,params,body,result;
    int anonymous = 0;
    int m;
    int ln,in;

    if (TRACE) printf("in functionDef...\n");

    f = match(ID);
    rethrow(f,0);
    assert(isIdentifier(f,"function"));
    ln = line(f);
    in = indent(f);

    if (check(ID))
        v = match(ID);
    else
        {
        v = newSymbol("anonymous");
        anonymous = 1;
        }

    push(v,"parser:functionDef:v");

    m = match(OPEN_PARENTHESIS);
    rethrow(m,1);
    
    params = optParamList();
    rethrow(params,1);

    push(params,"parser:functionDef:params");

    m = match(CLOSE_PARENTHESIS);
    rethrow(m,2);

    m = match(OPEN_BRACE);
    rethrow(m,2);

    body = nakedBlock();
    rethrow(body,2);

    push(body,"parser:functionDef:body");

    m = match(CLOSE_BRACE);
    rethrow(m,3);

    if (anonymous)
        {
        m = match(SEMI);
        rethrow(m,3);
        }

    ensureMemory(6);

    body = restore("parser:functionDef:body");
    params = restore("parser:functionDef:params");
    v = restore("parser:functionDef:v");

    if (car(body) == 0)
        {
        return throw("syntaxError",
            "blocks must contain at least one statement");
        }

    /*
    result = ucons(JOIN,v,0);
    result = ucons(JOIN,body,result);
    result = ucons(FUNCTION,params,result);
    result = ucons(FUNCTION_DEFINITION,v,result);
    */

    //indent the name so that we can find the indent after the call
    indent(v) = in;

    result = ucons(JOIN,body,0);
    result = ucons(JOIN,params,result);
    result = ucons(JOIN,v,result);
    result = ucons(CALL,newIdentifier("function"),result);

    line(result) = ln;
    indent(result) = in;

    if (TRACE) ppf("leaving functionDef: ",result,"\n");

    return result;
    }

int
parseIncludedFile(int f)
    {
    extern char *SwayLib;
    int result;
    FILE *oldInput,*fp;
    int oldIndenting,oldPending,oldPushedBack,oldPushBack;
    int (*oldReader)(FILE *);
    char orig[4024];

    cellString(orig,sizeof(orig),f);

    if (TRACE) printf("in parseIncluded file: <%s>\n", orig);

    fp = fopen(orig, "r");

    /* if no go, try the sway library */

    if (fp == 0) 
        {
        char buffer[4024];

        if (strlen(SwayLib) + count(f) >= sizeof(buffer) - 2)
            {
            return throw("includeError","include file name is too large");
            }
                
        strcpy(buffer,SwayLib);
        strcat(buffer,"/");
        strcat(buffer,orig);
        fp = fopen(buffer, "r");
        }

    /* if no go, try /usr/lib/sway */

    if (fp == 0) 
        {
        char buffer[4024];

        if (strlen("/usr/lib/sway") + count(f) >= sizeof(buffer) - 2)
            {
            return throw("includeError",
                "include file name is too large");
            }
                
        strcpy(buffer,"/usr/lib/sway");
        strcat(buffer,"/");
        strcat(buffer,orig);
        fp = fopen(buffer, "r");
        }

    if (fp == 0)
        {
        char buffer[4024];

        if (strlen("/usr/lib/sway/") + count(f) >= sizeof(buffer) - 2)
            {
            return throw("includeError",
                "include file name is too large");
            }
                
        strcpy(buffer,"/usr/lib/sway/");
        strcat(buffer,"/");
        strcat(buffer,orig);
        fp = fopen(buffer, "r");
        }

    if (fp == 0)
        {
        return throw("includeError",
            "include file not found: %s",
            orig);
        }

    oldInput = Input;
    oldPending = Pending;
    oldReader = getNextCharacter;
    oldPushedBack = pushedBack;
    oldPushBack = pushBack;
    oldIndenting = Indenting;

    Input = fp;
    getNextCharacter = fileGetChar;
    FileIndex = findSymbol(orig);
    LineNumber = 1;
    Indenting = 1;

    if (TRACE > 1) printf("parsing include %s\n",symbols[FileIndex]);

    clearDirective(Input);

    result = optDefinitionSeq();

    fclose(Input);

    if (type(result) != THROW)
        result = cons(NAKED_BLOCK,result,0);

    pushBack = oldPushBack;
    pushedBack = oldPushedBack;
    Input = oldInput;
    Pending = oldPending;
    getNextCharacter = oldReader;
    Indenting = oldIndenting;

    if (TRACE) ppf("leaving parseIncludedFile: ",result,"\n");

    return result;
    }
    
/* optStatementSeq : *empty*
 *                 | statement optStatementSeq
 */

static int
optStatementSeq()
    {
    int result;

    if (TRACE) printf("in optStatementSeq...\n");

    if (check(ID) && isIdentifier(Pending,"var"))
        {
        char buffer[512];
        errorContext(buffer,sizeof(buffer));
        return throw("syntaxError",
            "a variable definition is unexpected at this point\n%s",
            buffer);
        }

    if (isStatementPending())
        {
        int item,others;

        item = statement();
        rethrow(item,0);
        push(item,"parser:optStatementSeq:item");
        others = optStatementSeq();
        item = restore("parser:optStatementSeq:item");
        rethrow(others,0);
        result = cons(JOIN,item,others);
        }
    else
        {
        result = 0;
        }

    if (TRACE) ppf("leaving optStatementSeq: ",result,"\n");

    return result;
    }

/* statement : expr SEMI
 *           | block
 */

static int
statement()
    {
    int r,s;
    int m;

    if (TRACE) printf("in statement...\n");

    if (isBlockPending())
        {
        char buffer[512];
        errorContext(buffer,sizeof(buffer));
        return throw("syntaxError",
            "unattached block\n%s",
            buffer);
        }

    r = expr();

    rethrow(r,0);

    push(r,"parser:statement:r");
    s = cons(STATEMENT,r,0);
    r = restore("parser:statement:r");

    if (TRACE > 1) printf("statement subtype is %s\n", type(r));

    line(s) = line(r);
    indent(s) = indent(r);

    if (!isXCall(r))
        {
        push(s,"parser:statement:s");
        //ppf("statement is: ",r,"\n");
        m = match(SEMI);
        s = restore("parser:statement:s");
        rethrow(m,0);
        }

    if (TRACE) ppf("leaving statement: ",s,"\n");

    return s;
    }

/* expr : exprAssign */

static int
expr()
    {
    return exprAssign();
    }

/* exprAssign : exprConnect
 *            | exprConnect ID(=,tail=,head=) exprAssign
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

    push(a,"parser:exprAssign:a");
    if (opType() == UPDATER)
        {
        b = match(ID);
        rethrow(b,1);
        push(b,"parser:exprAssign:b");
        c = exprAssign();
        rethrow(c,2);
        push(c,"parser:exprAssign:c");
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

/* exprConnect : exprCompare (ID(<,<=,==,>=,>,!=) exprConnect)* 
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

    push(result,"parser:exprConnect:result");

    while (opType() == LOGICAL_CONNECTIVE)
        {
        b = match(ID);
        rethrow(b,1);
        push(b,"parser:exprConnect:b");
        c = exprCompare();
        rethrow(c,2);
        push(c,"parser:exprConnect:c");
        ensureMemory(3);
        c = restore("parser:exprConnect:c");
        b = restore("parser:exprConnect:b");
        a = restore("parser:exprConnect:result");
        result = ucons(JOIN,c,0);
        result = ucons(JOIN,a,result);
        result = ucons(BINARY,b,result);
        line(result) = line(a);
        indent(result) = indent(a);
        push(result,"parser:exprConnect:result");
        }

    result = restore("parser:exprConnect:result");

    if (TRACE) ppf("leaving exprConnect: ",result,"\n");

    return result;
    }

/* exprCompare : exprMath (ID(+,-,*,/,%) exprMath)*
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
    push(result,"parser:exprCompare:result");

    while (opType() == LOGICAL_COMPARISON)
        {
        b = match(ID);
        rethrow(b,1);
        push(b,"parser:exprCompare:b");
        c = exprMath();
        rethrow(c,2);
        push(c,"parser:exprCompare:c");
        ensureMemory(3);
        c = restore("parser:exprCompare:c");
        b = restore("parser:exprCompare:b");
        a = restore("parser:exprCompare:result");
        result = ucons(JOIN,c,0);
        result = ucons(JOIN,a,result);
        result = ucons(BINARY,b,result);
        line(result) = line(a);
        indent(result) = indent(a);
        push(result,"parser:exprCompare:result");
        }

    result = restore("parser:exprCompare:result");

    if (TRACE) ppf("leaving exprCompare: ",result,"\n");

    return result;
    }

/* exprMath : exprCall (ID exprCall)* */

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

    push(result,"parser:exprMath:result");

    while (opType() == ARITHMETIC)
        {
        b = match(ID);
        //ppf("exprMath: b was: ",b,"\n");
        rethrow(b,1);
        push(b,"parser:exprMath:b");
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

/* exprCall : exprSelect
 *          | exprSelect OPAREN optArgList CPAREN optExtraArgs
 *          | exprSelect OPAREN optArgList CPAREN optExtraArgs
 *              ID(.) exprCall
 */

static int
exprCall(int item)
    {
    int op,args,extra;
    int result;
    int m;

    if (TRACE) printf("in exprCall...\n");

    if (item == 0)
        op = exprSelect(0);
    else
        op = item;

    //ppf("exprCall: op was: ",op,"\n");
    rethrow(op,0);

    save(op,"parser:exprCall:op");

    if (check(OPEN_PARENTHESIS)) /* check is not gc-safe */
        {
        m = match(OPEN_PARENTHESIS);
        rethrow(m,1);
        args = optArgList();
        //ppf("exprCall: args was :",args,"\n");
        rethrow(args,1);
        save(args,"parser:exprCall:args");
        m = match(CLOSE_PARENTHESIS);
        rethrow(m,2);
        extra = optExtraArgs();
        rethrow(extra,2);
        save(extra,"parser:exprCall:extra");
        //ppf("exprCall: extra was :",extra,"\n");
        ensureMemory(1);
        extra = restore("parser:exprCall:extra");
        args = restore("parser:exprCall:args");
        op = restore("parser:exprCall:op");
        //ppf("exprCall: extra is :",extra,"\n");
        //ppf("exprCall: args is :",args,"\n");
        //ppf("exprCall: op is :",op,"\n");
        /* append is destructive, does not use cons */
        result = ucons(JOIN,op,append(args,extra));
        line(result) = line(op);
        indent(result) = indent(op);
        if (extra != 0)
            {
            type(result) = XCALL;
            }
        else
            {
            type(result) = CALL;
            }
        if (opType() == SELECT)
            {
            result = exprCall(exprSelect(result));
            }
        }
    else
        {
        op = restore("parser:exprCall:op");
        result = op;
        }

    if (TRACE) ppf("leaving exprCall: ",result,"\n");

    return result;
    }

/* exprSelect : primary [ID(.) primary]*
 *            | primary [OBRACKET expr CBRACKET]*
 */

static int
exprSelect(int item)
    {
    int a,b,c;
    int result;

    if (TRACE) printf("in exprSelect...\n");

    if (item == 0)
        a = primary();
    else
        a = item;

    rethrow(a,0);

    result = a;
    save(result,"parser:exprSelect:result");

    while (opType() == SELECT)
        {
        /* opType forces a lex */
        if (isIdentifier(Pending,OPEN_BRACKET))
            {
            b = match(ID);
            rethrow(b,1);
            save(b,"parser:exprSelect:b");
            c = expr();
            rethrow(c,2);
            save(c,"parser:exprSelect:c");
            match(CLOSE_BRACKET);
            }
        else /* must be dot */
            {
            b = match(ID);
            rethrow(b,1);
            save(b,"parser:exprSelect:b");
            if (check(ID))
            c = match(ID); //do not check for undefined here
            else
            c = primary();
            rethrow(c,2);
            save(c,"parser:exprSelect:c");
            }
        ensureMemory(3);
        c = restore("parser:exprSelect:c");
        b = restore("parser:exprSelect:b");
        a = restore("parser:exprSelect:result");
        result = ucons(JOIN,c,0);
        result = ucons(JOIN,a,result);
        result = ucons(BINARY,b,result);
        line(result) = line(a);
        indent(result) = indent(a);
        save(result,"parser:exprSelect:result");
        }

    result = restore("parser:exprSelect:result");

    if (TRACE) ppf("leaving exprSelect: ",result,"\n");

    return result;
    }

/* primary : INTEGER | REAL | STRING | SYMBOL
 *         | ID
 *         | ID(return) expr
 *         | functionExpr
 *         | parenExpr
 *         | block
 */

static int
primary()
    {
    int result;

    if (TRACE) printf("in primary...\n");

    if (check(INTEGER))
        result = match(INTEGER);
    else if (check(REAL))
        result = match(REAL);
    else if (check(STRING))
        result = match(STRING);
    else if (check(SYMBOL))
        result = match(SYMBOL);
    //else if (check(FUNCTION))
        //result = functionExpr();
    else if (check(ID))
        result = idExpr();
    else if (check(OPEN_PARENTHESIS))
        result = parenExpr();
    else if (check(OPEN_BRACE))
        result = block();
    else
        {
        char buffer[512];
        errorContext(buffer,sizeof(buffer));
        return throw("syntaxError",
            "an expression was expected, found %s instead\n%s",
            type(Pending),buffer);
        }

    if (TRACE) ppf("leaving primary: ",result,"\n");

    return result;
    }

static int
idExpr()
    {
    int result;
    int key = match(ID);

    rethrow(key,0);

    if (ival(key) == findSymbol("return"))
        result = returnExpr(key);
    else if (ival(key) == findSymbol("function"))
        result = functionExpr(key);
    else
        result = key;

    return result;
    }

static int
returnExpr(int key)
    {
    int result;

    /* return is converted into a function call */
    /* that will throw a special exception */
    save(key,"parser:primary:key");
    if (!isExprPending())
        {
        /* no return expression */
        result = newSymbol("void");
        }
    else
        {
        result = expr();
        }
    rethrow(result,1);
    result = cons(JOIN,result,0);
    key = restore("parser:primary:key");
    result = cons(CALL,key,result);
    indent(result) = indent(key);

    return result;
    }

/* functionExpr : FUNCTION OPAREN optParamList CPAREN OBRACE nakedBlock CBRACE
 */

static int
functionExpr(int key)
    {
    int params,body;
    int result;
    int m;

    if (TRACE) printf("in functionExpr...\n");

    if (check(ID))
        {
        int name = match(ID);
        char buffer[512];
        errorContext(buffer,sizeof(buffer));
        return throw("syntaxError",
            "misplaced definition of function %s\n%s",
            symbols[ival(name)],buffer);
        }

    m = match(OPEN_PARENTHESIS);
    rethrow(m,0);

    params = optParamList();
    rethrow(params,0);
    save(params,"parser:functionExpr:params");
    m = match(CLOSE_PARENTHESIS);
    rethrow(m,1);
    m = match(OPEN_BRACE);
    rethrow(m,1);
    body = nakedBlock();
    rethrow(body,1);
    save(body,"parser:functionExpr:body");
    m = match(CLOSE_BRACE);
    rethrow(m,2);

    ensureMemory(5);

    body = restore("parser:functionExpr:body");
    params = restore("parser:functionExpr:params");

    /* if you change this order, you must change make_closure */
    /* and the closure accessors in env.h */

    /*
    result = ucons(JOIN,newSymbol("anonymous"),0); // name
    result = ucons(JOIN,body,result);
    result = ucons(FUNCTION,params,result);
    */

    result = ucons(JOIN,body,0);
    result = ucons(JOIN,params,result);
    result = ucons(CALL,newIdentifier("lambda"),result);

    if (TRACE) ppf("leaving functionExpr: ",result,"\n");

    return result;
    }

/* parenExpr : OPAREN expr CPAREN */

static int
parenExpr()
    {
    int a;
    int result;
    int m;

    m = match(OPEN_PARENTHESIS);
    rethrow(m,0);

    a = expr();
    rethrow(a,0);

    save(a,"parser:parenExpr:a");
    m = match(CLOSE_PARENTHESIS);
    a = restore("parser:parenExpr:a");
    rethrow(m,0);

    result = cons(PARENTHESIZED_EXPR,a,0);

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

/* arg : expr
 *     | block
 */

static int
arg()
    {
    if (check(OPEN_BRACE))
        return block();
    else
        return expr();
    }

/* optExtraArgs : *empty*
 *              | extraArgs
 */

static int
optExtraArgs()
    {
    if (check(OPEN_BRACE))
        return extraArgs();
    else
        return 0;
    }

/* extraArgs : block
 *           | block ELSE block
 *           | block ELSE exprSelect OPAREN optArgList CPAREN extrArgs
 */

static int
extraArgs()
    {
    int a,b;
    int result;
    int m;

    if (TRACE) printf("in extraArgs...\n");

    a = block();
    rethrow(a,0);
    save(a,"parser:extraArgs:a");

    if (check(ALTERNATIVE)) //else /* check is not gc-safe */
        {
        m = match(ALTERNATIVE);
        rethrow(m,1);
        if (check(OPEN_BRACE))
            {
            b = block();
            rethrow(b,1);
            save(b,"parser:extraArgs:b");
            ensureMemory(2);
            b = restore("parser:extraArgs:b");
            a = restore("parser:extraArgs:a");
            result = ucons(JOIN,b,0);
            result = ucons(JOIN,a,result);
            }
        else  /* parse an extended call */
            {
            int op,args,extra;
            op = exprSelect(0);
            rethrow(op,1);
            save(op,"parser:extraArgs:op");
            m = match(OPEN_PARENTHESIS);
            rethrow(m,2);
            args = optArgList();
            rethrow(args,2);
            save(args,"parser:extraArgs:args");
            m = match(CLOSE_PARENTHESIS);
            rethrow(m,3);
            extra = extraArgs();
            rethrow(extra,3);
            save(extra,"parser:extraArgs:extra");
            ensureMemory(3);
            extra = restore("parser:extraArgs:extra");
            args = restore("parser:extraArgs:args");
            op = restore("parser:extraArgs:op");
            a = restore("parser:extraArgs:a");
            result = ucons(XCALL,op,append(args,extra));
            result = ucons(JOIN,result,0);
            result = ucons(JOIN,a,result);
            }
        }
    else
        {
        a = restore("parser:extraArgs:a");
        result = cons(JOIN,a,0);
        }

    if (TRACE) ppf("leaving extraArgs: ",result,"\n");

    return result;
    }

/* optParamList : paramList
 *              | *empty*
 */
  
static int
optParamList()
    {
    if (check(ID))
        return paramList();
    else
        return 0;
    }

/* paramList : ID
 *           | ID COMMA paramList
 */

static int
paramList()
    {
    int a,b;
    int result;

    if (TRACE) printf("in paramList...\n");

    a = match(ID);
    rethrow(a,0);

    save(a,"parser:paramList:a");

    if (check(COMMA)) /* check is not gc-safe */
        {
        match(COMMA);
        if (check(ID))
            b = paramList();
        else
            b = 0;
        a = restore("parser:paramList:a");
        rethrow(b,0);
        }
    else
        {
        a = restore("parser:paramList:a");
        b = 0;
        }

    result = cons(JOIN,a,b);

    if (TRACE) ppf("leaving paramList: ",result,"`\n");

    return result;
    }

/* block : OBRACE optDefinitionSeq optStatementSeq CBRACE */

static int
block()
    {
    int a,b,result;
    int m;

    if (TRACE) printf("in block...\n");

    m = match(OPEN_BRACE);
    rethrow(m,0);

    a = optDefinitionSeq();
    rethrow(a,0);

    save(a,"parser:block:a");
    b = optStatementSeq();
    rethrow(b,1);

    save(b,"parser:block:b");
    m = match(CLOSE_BRACE);
    rethrow(m,2);

    ensureMemory(1);
    b = restore("parser:block:b");
    a = restore("parser:block:a");

    if (a == 0 && b == 0)
        {
        return throw("syntaxError",
            "blocks must contain at least one statement");
        }

    result = ucons(BLOCK,append(a,b),0);
    if (a == 0) type(result) = NAKED_BLOCK;

    if (TRACE) ppf("leaving block: ",result,"\n");

    return result;
    }

/***** pending utilities ************************************************/

static int
isXCall(int e)
    {
    return type(e) == XCALL;
    }

static int
opType()
    {
    int result;

    //force lex
    check(0);

    if (type(Pending) != ID)
        {
        result = NOTanOP;
        }
    else
        {
        if (isIdentifier(Pending,ASSIGN)
        ||  isIdentifier(Pending,TAIL_ASSIGN)
        ||  isIdentifier(Pending,HEAD_ASSIGN))
            result = UPDATER;
        else if (isIdentifier(Pending,AND) || isIdentifier(Pending,OR))
            result = LOGICAL_CONNECTIVE;
        else if (isIdentifier(Pending,LT) || isIdentifier(Pending,GT)
        || isIdentifier(Pending,EQ) ||  isIdentifier(Pending,NEQ)
        || isIdentifier(Pending,GTE) || isIdentifier(Pending,LTE))
            result = LOGICAL_COMPARISON;
        else if (isIdentifier(Pending,DOT)
        || isIdentifier(Pending,OPEN_BRACKET))
            result = SELECT;
        else
            result = ARITHMETIC;
        }

    return result;
    }

static int
isStatementPending()
    {
    return isExprPending();
    }

static int
isDefinitionPending()
    {
    //return check(VAR) || check(FUNCTION) || isIdentifier(Pending,"include")
    check(ID);
    return isIdentifier(Pending,"var")
        || isIdentifier(Pending,"function")
        || isIdentifier(Pending,"include")
        || isIdentifier(Pending,"includeOnce");
    }

static int
isExprPending()
    {
    return 
        check(SYMBOL)
        || check(INTEGER) || check(REAL) || check(STRING)
        || check(OPEN_PARENTHESIS) || check(ID)
            || check(OPEN_BRACE);
    }

static int
isBlockPending()
    {
    return check(OPEN_BRACE);
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
