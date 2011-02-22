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
        if (transferred(items))
            {
            fprintf(fp," NEW(%d))",cdr(items));
            return;
            }
        items = cdr(items);
        if (items)
            {
            if (type(items) == CONS)
                fprintf(fp," ");
            else
                {
                fprintf(stdout," . ");
                ppLevel(fp,items,mode + 1);
                break;
                }
            }
        }
    fprintf(fp,")");
    }

void
ppObject(FILE *fp,int expr,int mode)
    {
    int vars = object_variables(expr);
    int vals = object_values(expr);

    if (sameSymbol(object_label(expr),thunkSymbol))
        fprintf(fp,"<thunk %d>",expr);
    else if (sameSymbol(object_label(expr),errorSymbol))
        fprintf(fp,"<error %d>",expr);
    else if (sameSymbol(object_label(expr),builtInSymbol))
        {
        fprintf(fp,"<builtIn ");
        ppLevel(fp,closure_name(expr),mode+1);
        ppList(fp,closure_parameters(expr),mode);
        fprintf(fp,">");
        }
    else if (sameSymbol(object_label(expr),closureSymbol))
        {
        fprintf(fp,"<function ");
        ppLevel(fp,closure_name(expr),mode+1);
        ppList(fp,closure_parameters(expr),mode);
        fprintf(fp,">");
        }
    else
        {
        fprintf(fp,"<");
        ppLevel(fp,car(expr),mode);
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
                ppLevel(fp,car(vals),mode+1);
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
    }
        
void ppCons(FILE *fp,int expr,int mode)
    {
    int old = ppQuoting;

    ppQuoting = 1;
    if (sameSymbol(car(expr),objectSymbol))
        ppObject(fp,expr,mode);
    else if (sameSymbol(car(expr),lambdaSymbol))
        {
        fprintf(fp,"<lambda ");
        ppList(fp,closure_parameters(expr),mode);
        fprintf(fp,">");
        }
    else if (transferred(car(expr)))
        fprintf(fp,"(XXX ...)");
    else
        ppList(fp,expr,mode);

    ppQuoting = old;
    }

void
ppString(FILE *fp,int expr,int mode)
    {
    if (ppQuoting) fprintf(fp,"\"");
    while (expr != 0)
        {
        fprintf(fp,"%c",ival(expr));
        if (transferred(expr))
            {
            fprintf(fp,"...NEW(%d)\"",cdr(expr));
            return;
            }
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

    if (transferred(expr)) fprintf(fp,"*");
    }

void
debug(char *s,int i)
    {
    printf("%s: ",s);
    ppLevel(stdout,i,0);
    printf("\n");
    }
