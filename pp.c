#include <stdio.h>
#include "types.h"
#include "cell.h"
#include "pp.h"
#include "util.h"

void
ppList(FILE *fp,int expr)
    {
    if (expr == 0)
        fprintf(fp,"nil");
    else
        {
        fprintf(fp,"(");
        while (expr != 0)
            {
            pp(fp,car(items));
            expr:wq

            fprintf("

void
pp(FILE *fp,int expr)
    {
    if (type(expr) == INTEGER)
        {
        fprintf(fp,"%d",ival(expr));
        }
    else if (type(expr) == CONS)
        {
        ppList(fp,expr);
        }
    else
        {
        Fatal("pretty printing: type %s unknown\n",type(expr));
        }
    }
