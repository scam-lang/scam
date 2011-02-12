#include <stdio.h>
#include "types.h"
#include "cell.h"
#include "pp.h"
#include "env.h"
#include "util.h"

int ppQuoting = 0;

void
ppList(FILE *fp,int items,int mode)
    {
    fprintf(fp,"(");
    while (items != 0)
        {
        ppLevel(fp,car(items),mode + 1);
        items = cdr(items);
        if (items)
            fprintf(fp," ");
        }
    fprintf(fp,")");
    }

void
ppObject(FILE *fp,int expr,int mode)
    {
    int vars = object_variables(expr);
    int vals = object_values(expr);

    fprintf(fp,"<object %d>",expr);
    if (mode < 1)
        {
        fprintf(fp,"\n");
        while (vars != 0)
            {
            fprintf(fp,"%20s : ",SymbolTable[ival(car(vars))]);
            ppLevel(fp,car(vals),mode+1);
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
    if (sameSymbol(car(expr),objectSymbol))
        ppObject(fp,expr,mode);
    else if (sameSymbol(car(expr),thunkSymbol))
        fprintf(fp,"<thunk %d>",expr);
    else if (sameSymbol(car(expr),errorSymbol))
        fprintf(fp,"<error %d>",expr);
    else if (sameSymbol(car(expr),lambdaSymbol))
        {
        fprintf(fp,"<lambda ");
        ppList(fp,closure_parameters(expr),mode);
        fprintf(fp,">");
        }
    else if (sameSymbol(car(expr),builtInSymbol))
        {
        fprintf(fp,"<builtIn ");
        ppLevel(fp,closure_name(expr),mode+1);
        ppList(fp,closure_parameters(expr),mode);
        fprintf(fp,">");
        }
    else if (sameSymbol(car(expr),closureSymbol))
        {
        fprintf(fp,"<function ");
        ppLevel(fp,closure_name(expr),mode+1);
        ppList(fp,closure_parameters(expr),mode);
        fprintf(fp,">");
        }
    else
        ppList(fp,expr,mode);

    ppQuoting = old;
    }

void
ppString(FILE *fp,int expr,int mode)
    {
    if (ppQuoting) fprintf(fp,"\"");
    while (ival(expr) != 0)
        {
        fprintf(fp,"%c",ival(expr));
        expr = cdr(expr);
        }
    if (ppQuoting) fprintf(fp,"\"");
    }

void
pp(FILE *fp,int expr)
    {
    ppLevel(fp,expr,0);
    }

void
ppLevel(FILE *fp,int expr,int mode)
    {
    if (expr == 0)
        fprintf(fp,"nil");
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
    else
        Fatal("pretty printing: type %s unknown\n",type(expr));
    }

void
debug(char *s,int i)
    {
    printf("%s: ",s);
    pp(stdout,i);
    printf("\n");
    }
