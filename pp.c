
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
#include "cell.h"   /* include util.h, thread.h */
#include "env.h"    /* includes pp.h */
#include "pp-base.h"

#define min(a,b) (a < b? a : b)

int ppActual = 1;
int ppQuoting = 0;

static void ppLevel(int,int);

static void
ppList(char *open,int items,char *close,int level)
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
                ppPutChar(' ');
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
ppArray(char *open,int items,char *close,int level)
    {
    int size = count(items);
    ppPutString(open);
    while (size != 0)
        {
        ppLevel(car(items),level + 1);
        if (cdr(items) != 0)
            {
            ppPutChar(' ');
            }
        ++items;
        --size;
        }
    ppPutString(close);
    }

static void
ppObject(int expr,int level)
    {
    int address;

    if (Debugging)
        address = 0;
    else
        address = expr;

    if (SameSymbol(object_label(expr),ThunkSymbol))
        {
        ppPutString("<thunk ");
        ppPutInt(address);
        ppPutChar('>');
        }
    else if (SameSymbol(object_label(expr),ErrorSymbol))
        {
        ppPutString("<error ");
        ppLevel(error_code(expr),level+1);
        ppPutInt(address);
        ppPutChar('>');
        }
    else if (SameSymbol(object_label(expr),BuiltInSymbol))
        {
        if (ppActual)
            {
            ppPutString("<builtin ");
            ppLevel(closure_name(expr),level+1);
            ppList("(",closure_parameters(expr),")",level);
            ppPutChar('>');
            }
        else
            ppLevel(closure_name(expr),level+1);
        }
    else if (SameSymbol(object_label(expr),ClosureSymbol))
        {
        if (ppActual)
            {
            ppPutString("<function ");
            ppLevel(closure_name(expr),level+1);
            ppList("(",closure_parameters(expr),")",level);
            ppPutChar('>');
            }
        else
            ppLevel(closure_name(expr),level+1);
        }
    else if (env_constructor(expr) == 0)
        {
        ppPutString("<environment ");
        ppPutInt(address);
        ppPutChar('>');
        }
    else
        {
        ppPutString("<object ");
        ppLevel(closure_name(env_constructor(expr)),level+1);
        ppPutInt(address);
        ppPutChar('>');
        }
    }

static void
ppCons(int expr,int level)
    {
    int old = ppQuoting;

    ppQuoting = 1;
    if (SameSymbol(car(expr),ObjectSymbol))
        ppObject(expr,level);
    else
        ppList("(",expr,")",level);

    ppQuoting = old;
    }

static void
ppFormattedString(int expr)
    {
    int size = count(expr);
    if (ppQuoting) ppPutChar('\"');
    while (size != 0)
        {
        ppPutChar(ival(expr));
        ++expr;
        --size;
        }
    if (ppQuoting) ppPutChar('\"');
    }

void
scamPPFile(FILE *fp,int expr)
    {
    ppToFile(fp,0);
    ppLevel(expr,0);
    }

void
scamPPString(char *buffer,int size,int expr)
    {
    ppToString(buffer,size);
    ppLevel(expr,0);
    }

static void
ppLevel(int expr,int level)
    {
    if (expr == 0)
        ;//fprintf(fp,"nil");
    else if (type(expr) == INTEGER)
        ppPutInt(ival(expr));
    else if (type(expr) == REAL)
        ppPutReal(rval(expr));
    else if (type(expr) == STRING)
        ppFormattedString(expr);
    else if (type(expr) == SYMBOL)
        ppPutString(SymbolTable[ival(expr)]);
    else if (type(expr) == CONS)
        ppCons(expr,level);
    else if (type(expr) == ARRAY)
        ppArray("[",expr,"]",level);
    else
        ppPutString(type(expr));
    }

void
scamppToString(int expr,char *buffer,int size,int *index)
    {
    char local[1024];
    if (type(expr) == CONS)
        {
        buffer[*index] = '(';
        *index += 1;
        while (expr != 0)
           {
           scamppToString(car(expr),buffer,size,index);
           expr = cdr(expr);
           if (expr != 0)
                {
                buffer[*index] = ' ';
                *index += 1;
                }
           }
        buffer[*index] = ')';
        *index += 1;
        buffer[*index] = '\0';
        }
    else if (type(expr) == SYMBOL)
        {
        strcpy(buffer+*index,SymbolTable[ival(expr)]);
        *index += strlen(SymbolTable[ival(expr)]);
        buffer[*index] = '\0';
        }
    else if (type(expr) == INTEGER)
        {
        sprintf(local,"%d",ival(expr));
        strcpy(buffer+*index,local);
        *index += strlen(local);
        buffer[*index] = '\0';
        }
    else if (type(expr) == REAL)
        {
        sprintf(local,"%.17f",rval(expr));
        strcpy(buffer+*index,local);
        *index += strlen(local);
        buffer[*index] = '\0';
        }
    else if (type(expr) == STRING)
        {
        buffer[*index] = '"';
        *index += 1;
        cellString(local,sizeof(local),expr);
        strcpy(buffer+*index,local);
        *index += strlen(local);
        buffer[*index] = '"';
        *index += 1;
        buffer[*index] = '\0';
        }
    else
        Fatal("INTERNAL ERROR: unknown ppToString type: %s\n",type(expr));
    }

void
debug(char *s,int i)
    {
    if (s != 0)
        printf("%s: ",s);
    if (Syntax == SWAY)
        swayPPFile(stdout,i);
    else
        scamPPFile(stdout,i);
    printf("\n");
    }

void 
debugOut(FILE *f, char *s, int i)
    {
    if (s != 0)
        fprintf(f, "%s: ",s);
    if (Syntax == SWAY)
        swayPPFile(stdout,i);
    else
        scamPPFile(stdout,i);
    fprintf(f,"\n");
    }

void
swayPPFile(FILE *fp,int expr)
    {
    scamPPFile(fp,expr);
    }

void
swayPPString(char *buffer,int size,int expr)
    {
    scamPPString(buffer,size,expr);
    }

void
ppTable(int expr,int level,int span)
    {
    if (!isObject(expr))
        {
        ppLevel(expr,level);
        ppPutChar('\n');
        return;
        }

    int vars = object_variables(expr);
    int vals = object_values(expr);

    ppPutString("<object");
    ppPutChar(' ');
    ppPutInt(expr);
    ppPutChar('>');

    if (level < 1)
        {
        ppPutChar('\n');
        while (vars != 0)
            {
            char buffer[512];
            snprintf(buffer,sizeof(buffer),"%24s",
                 SymbolTable[ival(car(vars))]);
            ppPutString(buffer);
            ppPutString(" : ");
            if (car(vals) == 0)
                ppPutString("nil");
            else
                {
                int oldQuoting = ppQuoting;
                int oldLength = ppLength;
                int oldMaxLength = ppMaxLength;
                int oldFlat = ppFlat;
                ppQuoting = 1;
                ppFlat = 1;
                ppLength = 0;
                if (span != 0)
                    ppMaxLength = span;
                ppLevel(car(vals),level+1);
                ppFlat = oldFlat;
                ppLength = oldLength + ppLength;
                ppMaxLength = oldMaxLength;
                ppQuoting = oldQuoting;
                }
            ppPutChar('\n');
            vars = cdr(vars);
            vals = cdr(vals);
            }
        }
    }
        
