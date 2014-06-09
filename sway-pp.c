
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

static void ppLevel(FILE *,int,int);

#ifdef HI

#define cfprintf(fp,str,value,spot) \
    do \
        { \
        if (status(spot) == MARKED) fprintf(fp,COLOR_START); \
        fprintf(fp,str,value); \
        if (status(spot) == MARKED) fprintf(fp,COLOR_END); \
        } \
    while (0)

#define Cfprintf(fp,str,spot) \
    do \
        { \
        if (status(spot) == MARKED) fprintf(fp,COLOR_START); \
        fprintf(fp,str); \
        if (status(spot) == MARKED) fprintf(fp,COLOR_END); \
        } \
    while (0)
#else



#define cfprintf(fp,str,value,spot) \
    do \
        { \
        fprintf(fp,str,value); \
        } \
    while (0)

#define Cfprintf(fp,str,spot) \
    do \
        { \
        fprintf(fp,str); \
        } \
    while (0)

#endif

static void
ppList(FILE *fp,char *open,int items,char *close,int mode)
    {
    cfprintf(fp,"%s",open,items);
    while (items != 0)
        {
        ppLevel(fp,car(items),mode + 1);
        items = cdr(items);
        if (items)
            {
            if (type(items) == CONS)
                {
                fprintf(fp," ");
                }
            else
                {
                Cfprintf(fp," . ",items);
                ppLevel(fp,items,mode + 1);
                break;
                }
            }
        }
    cfprintf(fp,"%s",close,items);
    }

static void
ppArray(FILE *fp,char *open,int items,char *close,int mode)
    {
    int size = count(items);
    cfprintf(fp,"%s",open,items);
    while (size != 0)
        {
        ppLevel(fp,car(items),mode + 1);
        if (cdr(items) != 0)
            {
            fprintf(fp," ");
            }
        ++items;
        --size;
        }
    cfprintf(fp,"%s",close,items);
    }

static void
ppObject(FILE *fp,int expr,int mode)
    {
    int address;
    if (Debugging)
        address = 0;
    else
        address = expr;

    if (SameSymbol(object_label(expr),ThunkSymbol))
        {
        cfprintf(fp,"<thunk %d>",address,expr);
        }
    else if (SameSymbol(object_label(expr),ErrorSymbol))
        {
        Cfprintf(fp,"<error ",expr);
        ppLevel(fp,error_code(expr),mode+1);
        cfprintf(fp," %d>",address,expr);
        }
    else if (SameSymbol(object_label(expr),BuiltInSymbol))
        {
        if (ppActual)
            {
            Cfprintf(fp,"<builtIn ",expr);
            ppLevel(fp,closure_name(expr),mode+1);
            ppList(fp,"(",closure_parameters(expr),")",mode);
            Cfprintf(fp,">",expr);
            }
        else
            ppLevel(fp,closure_name(expr),mode+1);
        }
    else if (SameSymbol(object_label(expr),ClosureSymbol))
        {
        if (ppActual)
            {
            Cfprintf(fp,"<function ",expr);
            ppLevel(fp,closure_name(expr),mode+1);
            ppList(fp,"(",closure_parameters(expr),")",mode);
            Cfprintf(fp,">",expr);
            }
        else
            ppLevel(fp,closure_name(expr),mode+1);
        }
    else if (env_constructor(expr) == 0)
        {
        cfprintf(fp,"<environment %d>",address,expr);
        }
    else
        {
        Cfprintf(fp,"<object ",expr);
        ppLevel(fp,closure_name(env_constructor(expr)),mode+1);
        cfprintf(fp," %d>",address,expr);
        }
    }

static void
ppCons(FILE *fp,int expr,int mode)
    {
    int old = ppQuoting;

    ppQuoting = 1;
    if (SameSymbol(car(expr),ObjectSymbol))
        ppObject(fp,expr,mode);
    else
        ppList(fp,"(",expr,")",mode);

    ppQuoting = old;
    }

static void
ppString(FILE *fp,int expr,int mode)
    {
    int size = count(expr);
    if (ppQuoting) Cfprintf(fp,"\"",expr);
    while (size != 0)
        {
        cfprintf(fp,"%c",ival(expr),expr);
        ++expr;
        --size;
        }
    if (ppQuoting) Cfprintf(fp,"\"",expr);
    }

void
swayPP(FILE *fp,int expr)
    {
    ppLevel(fp,expr,0);
    }

static void
ppLevel(FILE *fp,int expr,int mode)
    {
    if (expr == 0)
        ;//fprintf(fp,"nil");
    else if (type(expr) == INTEGER)
        cfprintf(fp,"%d",ival(expr),expr);
    else if (type(expr) == REAL)
        cfprintf(fp,"%f",rval(expr),expr);
    else if (type(expr) == STRING)
        ppString(fp,expr,mode);
    else if (type(expr) == SYMBOL)
        cfprintf(fp,"%s",SymbolTable[ival(expr)],expr);
    else if (type(expr) == CONS)
        ppCons(fp,expr,mode);
    else if (type(expr) == ARRAY)
        ppArray(fp,"[",expr,"]",mode);
    else
        printf("%s",type(expr));
    }

void
swayppToString(int expr,char *buffer,int size,int *index)
    {
    char local[1024];
    if (type(expr) == CONS)
        {
        buffer[*index] = '(';
        *index += 1;
        while (expr != 0)
           {
           swayppToString(car(expr),buffer,size,index);
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
