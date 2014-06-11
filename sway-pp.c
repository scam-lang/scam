
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
extern void ppFormattedString(char *);

static void ppBlock(int);
void ppSeq(int);
static void ppVarDef(int);
static void ppVarDefList(int);
static void ppStatement(int);
static void ppCall(int);
static void ppBinary(int);
static void ppPrintIndent(void);
static void ppString(int);
static void ppParenthesizedExpr(int);
static void ppJoin(int);
static void ppClosure(int);
static void ppXArgs(int);
static void ppCSV(int);
static void ppComplex(int,int);

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
ppComplex(int t,int level)
    {
    ppList("(",t,")",level+1);
    }
