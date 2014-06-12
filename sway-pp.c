
/*
 *  Main Author : John C. Lusth
 *  Barely Authors : Jeffrey Robinson, Gabriel Loewen
 *  Last Modified : May 4, 2014
 *
 *  TODO : Description
 *
 */

#include <string.h>

#include "scam.h"
#include "types.h"
#include "cell.h"
#include "env.h"    
#include "pp-base.h"

#define MAX_CSV 10

extern void ppList(char *,int,char *,int);
extern void ppFormattedString(int);
static void ppComplex(int,int);
static void ppLevel(int,int);
static int  isXCall(int);
static int  isBinary(int);

static int
isXCall(int expr)
    {
    return 0;
    }

int
isBinary(int sym)
    {
    if (type(sym) != SYMBOL) return 0;
    else return strchr("+-*/%^<>=!&|",SymbolTable[ival(sym)][0]) != 0;
    }

void
swayPPFile(FILE *fp,int expr)
    {
    ppToFile(fp,0);
    ppLevel(expr,0);
    }

void
swayPPString(char *buffer,int size,int expr)
    {
    printf("generating string\n");
    ppToString(buffer,size);
    ppLevel(expr,0);
    }

static void
ppSwayList(char *open,int items,char *close,int level)
    {
    ppPutString(open);
    while (items != 0)
        {
        ppLevel(car(items),level + 1);
        items = cdr(items);
        if (items)
            {
            if (type(items) == CONS)
                {
                ppPutString(",");
                }
            else
                {
                ppPutString(" . ");
                ppLevel(items,level + 1);
                break;
                }
            }
        }
    ppPutString(close);
    }

static void
ppLevel(int t,int level)
    {
    if (type(t) == END_OF_INPUT)
        ppPutString("END_OF_INPUT");
    else if (type(t) == INTEGER)
        ppPutInt(ival(t));
    else if (type(t) == REAL)
        ppPutReal(rval(t));
    else if (type(t) == STRING)
        ppFormattedString(t);
    else if (type(t) == SYMBOL)
        ppPutString(SymbolTable[ival(t)]);
    else if (type(t) == CONS)
        ppComplex(t,level+1);
    /*
    else if (type(t) == VARIABLE_DEFINITION_LIST)
        ppVarDefList(t);
    else if (type(t) == VARIABLE_DEFINITION)
        ppVarDef(t);
    else if (type(t) == STATEMENT)
        ppStatement(t);
    else if (type(t) == CALL || type(t) == XCALL)
        ppCall(t);
    else if (type(t) == BINARY)
        ppBinary(t);
    else if (type(t) == PARENTHESIZED_EXPR)
        ppParenthesizedExpr(t);
    else if (type(t) == ARRAY)
        ppList("[",t,"]",level+1);
    else if (type(t) == CONS)
        ppCons(t);
    */
    else
        {
        ppPutString("<");
        ppPutString(type(t));
        ppPutString(" ");
        ppPutInt(t);
        ppPutString(">");
        }
    }

static void
ppBlock(int body,int level)
    {
    if (ppFlat)
        ppPutChar(' ');
    else
        {
        ppIndent += 4;
        ppPutChar('\n');
        ppPutIndent();
        }
    ppPutChar('{');
    while (body != 0)
        {
        if (ppFlat)
            ppPutChar(' ');
        else
            {
            ppPutChar('\n');
            ppPutIndent();
            }
        ppLevel(car(body),level+1);
        if (!isXCall(car(body))) ppPutChar(';');
        body = cdr(body);
        }
    if (ppFlat)
        ppPutString(" }");
    else
        {
        ppPutChar('\n');
        ppPutIndent();
        ppPutString("}\n");
        ppIndent -= 4;
        }
    }

static void
ppVarDefinition(int t,int level)
    {
    ppList("(",t,")",level);
    }

static void
ppFunctionDefinition(int t,int level)
    {
    int body;
    ppPutString("function ");
    ppPutString(SymbolTable[ival(car(cadr(t)))]);
    ppSwayList("(",cdr(cadr(t)),")",level+1);
    ppBlock(cddr(t),level);
    }

static void
ppDefinition(int t,int level)
    {
    if (type(cadr(t)) == CONS)
        ppFunctionDefinition(t,level);
    else
        ppVarDefinition(t,level);
    }

static void
ppBinary(int t,int level)
    {
    ppLevel(cadr(t),level+1);
    ppPutChar(' ');
    ppLevel(car(t),level+1);
    ppPutChar(' ');
    ppLevel(caddr(t),level+1);
    }

static void
ppComplex(int t,int level)
    {
    printf("the length is %d\n",length(t));
    if (SameSymbol(car(t),DefineSymbol))
        ppDefinition(t,level);
    else if (SameSymbol(car(t),ParenSymbol))
        {
        ppPutChar('(');
        ppLevel(cadr(t),level+1);
        ppPutChar(')');
        }
    else if (length(t) == 3 && isBinary(car(t)))
        ppBinary(t,level);
    else
        {
        ppLevel(car(t),level+1);
        ppSwayList("(",cdr(t),")",level+1);
        }
    }
