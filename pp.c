#include <stdio.h>
#include "types.h"
#include "cell.h"
#include "pp.h"
#include "env.h"
#include "util.h"

int ppQuoting = 0;

static void ppLevel(FILE *,int,int);

void
ppList(FILE *fp,char *open,int items,char *close,int mode)
    {
    fprintf(fp,"%s",open);
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
                fprintf(fp," . ");
                ppLevel(fp,items,mode + 1);
                break;
                }
            }
        }
    fprintf(fp,"%s",close);
    }

void
ppArray(FILE *fp,char *open,int items,char *close,int mode)
    {
    int size = count(items);
    fprintf(fp,"%s",open);
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
    fprintf(fp,"%s",close);
    }

void
ppObject(FILE *fp,int expr,int mode)
    {
    if (sameSymbol(object_label(expr),thunkSymbol))
        fprintf(fp,"<thunk %d>",expr);
    else if (sameSymbol(object_label(expr),errorSymbol))
        {
        fprintf(fp,"<error ");
        ppLevel(fp,error_code(expr),mode+1);
        fprintf(fp," %d>",expr);
        }
    else if (sameSymbol(object_label(expr),builtInSymbol))
        {
        fprintf(fp,"<builtIn ");
        ppLevel(fp,closure_name(expr),mode+1);
        ppList(fp,"(",closure_parameters(expr),")",mode);
        fprintf(fp,">");
        }
    else if (sameSymbol(object_label(expr),closureSymbol))
        {
        fprintf(fp,"<function ");
        ppLevel(fp,closure_name(expr),mode+1);
        ppList(fp,"(",closure_parameters(expr),")",mode);
        fprintf(fp,">");
        }
    else if (env_constructor(expr) == 0)
        {
        fprintf(fp,"<environment %d>",expr);
        }
    else
        {
        fprintf(fp,"<object ");
        ppLevel(fp,closure_name(env_constructor(expr)),mode+1);
        fprintf(fp," %d>",expr);
        }
    }

void
ppTable(FILE *fp,int expr,int mode)
    {
    int vars = object_variables(expr);
    int vals = object_values(expr);

    fprintf(fp,"<object");
    fprintf(fp," %d>",expr);
    if (mode < 1)
        {
        fprintf(fp,"\n");
        while (vars != 0)
            {
            fprintf(fp,"%20s",SymbolTable[ival(car(vars))]);
            if (transferred(car(vars)))
                fprintf(fp,"* : ");
            else
                fprintf(fp,"  : ");
            if (car(vals) == 0)
                fprintf(fp,"nil");
            else
                {
                int old = ppQuoting;
                ppQuoting = 1;
                ppLevel(fp,car(vals),mode+1);
                ppQuoting = old;
                }
            fprintf(fp,"\n");
            if (transferred(vars) || transferred(vals))
                {
                fprintf(fp,"xxxxxxxxxxxxxxxxxxxxx\n");
                return;
                }
            vars = cdr(vars);
            vals = cdr(vals);
            }
        }
    }
        
void ppCons(FILE *fp,int expr,int mode)
    {
    int old = ppQuoting;

    ppQuoting = 1;
    if (sameSymbol(car(expr),objectSymbol))
        ppObject(fp,expr,mode);
    else
        ppList(fp,"(",expr,")",mode);

    ppQuoting = old;
    }

void
ppString(FILE *fp,int expr,int mode)
    {
    int size = count(expr);
    if (ppQuoting) fprintf(fp,"\"");
    while (size != 0)
        {
        fprintf(fp,"%c",ival(expr));
        if (transferred(expr))
            {
            fprintf(fp,"[NEW(%d)]",cdr(expr));
            }
        ++expr;
        --size;
        }
    if (ppQuoting) fprintf(fp,"\"");
    }

void
scamPP(FILE *fp,int expr)
    {
    ppLevel(fp,expr,0);
    }

static void
ppLevel(FILE *fp,int expr,int mode)
    {
    if (expr == 0)
        ;//fprintf(fp,"");
    else if (type(expr) == INTEGER)
        fprintf(fp,"%d",ival(expr));
    else if (type(expr) == REAL)
        fprintf(fp,"%f",rval(expr));
    else if (type(expr) == STRING)
        ppString(fp,expr,mode);
    else if (type(expr) == SYMBOL)
        fprintf(fp,"%s",SymbolTable[ival(expr)]);
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

    if (transferred(expr)) fprintf(fp,"*");
    }

void
debug(char *s,int i)
    {
    if (s != 0)
        printf("%s: ",s);
    ppLevel(stdout,i,0);
    printf("\n");
    }
