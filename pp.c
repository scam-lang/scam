
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

int ppActual = 1;
int ppQuoting = 0;

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

void
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

void
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

void
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

void
ppTable(FILE *fp,int expr,int mode)
    {
    if (!isObject(expr))
        {
        ppLevel(fp,expr,0);
        fprintf(fp,"\n");
        return;
        }

    int vars = object_variables(expr);
    int vals = object_values(expr);

    Cfprintf(fp,"<object",expr);
    if (Debugging)
        cfprintf(fp," %d>",expr,expr);
        //cfprintf(fp," %d>",0,expr);
    else
        cfprintf(fp," %d>",expr,expr);
    if (mode < 1)
        {
        fprintf(fp,"\n");
        while (vars != 0)
            {
            cfprintf(fp,"%30s",SymbolTable[ival(car(vars))],car(vars));
            Cfprintf(fp,"  : ",car(vars));
            if (car(vals) == 0)
                Cfprintf(fp,"nil",car(vals));
            else
                {
                int old = ppQuoting;
                ppQuoting = 1;
                ppLevel(fp,car(vals),mode+1);
                ppQuoting = old;
                }
            fprintf(fp,"\n");
            vars = cdr(vars);
            vals = cdr(vals);
            }
        }
    }
        
void ppCons(FILE *fp,int expr,int mode)
    {
    int old = ppQuoting;

    ppQuoting = 1;
    if (SameSymbol(car(expr),ObjectSymbol))
        ppObject(fp,expr,mode);
    else
        ppList(fp,"(",expr,")",mode);

    ppQuoting = old;
    }

void
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
scamPP(FILE *fp,int expr)
    {
    ppLevel(fp,expr,0);
    }

void
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
    else if (type(expr) == PAST)
        fprintf(fp,"!PAST!");
    else if (type(expr) == FUTURE)
        fprintf(fp,"!FUTURE!");
    else
        printf("%s",type(expr));
    }

void
ppToString(int expr,char *buffer,int size,int *index)
    {
    char local[1024];
    if (type(expr) == CONS)
        {
        buffer[*index] = '(';
        *index += 1;
        while (expr != 0)
           {
           ppToString(car(expr),buffer,size,index);
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
    ppLevel(stdout,i,0);
    printf("\n");
    }

void 
debugOut(FILE *f, char *s, int i)
    {
    if (s != 0)
        fprintf(f, "%s: ",s);
    ppLevel(f,i,0);
    fprintf(f,"\n");
    }
