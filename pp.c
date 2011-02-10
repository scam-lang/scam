#include <stdio.h>
#include "types.h"
#include "cell.h"
#include "pp.h"
#include "util.h"

void
ppList(FILE *fp,int items)
    {
    fprintf(fp,"(");
    while (items != 0)
        {
        pp(fp,car(items));
        items = cdr(items);
        if (items)
            fprintf(fp," ");
        }
    fprintf(fp,")");
    }

void
ppObject(FILE *fp,int expr)
    {
    fprintf(fp,"<object %d>\n",expr);
    }

void
pp(FILE *fp,int expr)
    {
    if (expr == 0)
        fprintf(fp,"nil");
    else if (type(expr) == INTEGER)
        fprintf(fp,"%d",ival(expr));
    else if (type(expr) == REAL)
        fprintf(fp,"%f",rval(expr));
    else if (type(expr) == SYMBOL)
        fprintf(fp,"%s",SymbolTable[ival(expr)]);
    else if (type(expr) == CONS && sameSymbol(car(expr),objectSymbol))
        ppObject(fp,expr);
    else if (type(expr) == CONS)
        ppList(fp,expr);
    else
        Fatal("pretty printing: type %s unknown\n",type(expr));
    }
