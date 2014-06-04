#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "cell.h"
#include "types.h"
#include "parser.h"
#include "eval.h"
#include "env.h"
#include "pp.h"
#include "prim.h"

void
swayPP(FILE *fp,int expr)
    {
    swayPPLevel(fp,expr,0);
    }

void
swayPPLevel(FILE *fp,int expr,int mode)
    {
    printf("in swayPPLevel...\n");
    if (expr == 0)
        ; //fprintf(fp,"nil");
    else if (type(expr) == INTEGER)
        fprintf(fp,"%d",ival(expr));
    else if (type(expr) == REAL)
        fprintf(fp,"%f",rval(expr));
    else if (type(expr) == STRING)
        ppString(fp,expr,mode);
    else if (type(expr) == SYMBOL)
        fprintf(fp,"%s",SymbolTable[ival(expr)]);
    /*
    else if (type(expr) == ARRAY)
        ppArray(fp,"[",expr,"]",mode);
    else if (type(expr) != CONS)
        fprintf(fp,"%s",type(expr));
    else if (sameSymbol(expr,beginSymbol))
        ppBlock(fp,expr,mode);
    else if (sameSymbol(expr,defineSymbol))
        ppDefine(fp,expr,mode);
    else if (sameSymbol(expr,fillerSymbol)) // xcall
        ppXCall(fp,expr,mode);
    else
    */
        printf("%s",type(expr));
    }
/*
        ppBlock(t);
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
    else if (type(t) == PRIMITIVE)
        {
        ppPutString("<function ");
        ppPutString(primitives[ival(car(t))].name);
        if (cdr(t))
            pp(cdr(t));
        else
            ppPutString("()");
        ppPutString(">");
        }
    else if (type(t) == UNINITIALIZED) 
        ppPutString(UNINITIALIZED);
    else if (type(t) == ARRAY)
        ppArray(t);
    else if (type(t) == JOIN)
        ppJoin(t);
    else if (type(t) == CLOSURE && (ppFlags & ppOBJECT))
        ppEnv(t);
    else if (type(t) == CLOSURE)
        ppClosure(t);
    else if (type(t) == THUNK)
        ppEnv(t);
    else if (type(t) == ERROR)
        ppEnv(t);
    else if (type(t) == THROW)
        ppEnv(t);
    else if (type(t) == ENV)
        ppEnv(t);
    else if (type(t) == GONE)
        {
        ppPutString("<GONE ");
        ppPutInt(cdr(t));
        ppPutString(">");
        }
    else
        {
        ppPutString("<");
        //printf("type(t) is  %s\n",type(t));
        ppPutString(type(t));
        ppPutString(" ");
        ppPutInt(t);
        ppPutString(">");
        }
    }

static void
ppString(int t)
    {
    if (ppFlags & ppQUOTES)
        {
        ppPutString("\"");
        ppPutString(cellString(0,0,t));
        ppPutString("\"");
        }
    else
        {
        char *s = cellString(0,0,t);
        while (*s != '\0')
            {
            ppPutChar(*s);
            ++s;
            }
        }
    }

static void
ppSymbol(int t)
    {
    //printf("{SYMBOL %d}",t);
    if (ppFlags & ppQUOTES)
        ppPutString(":");
    ppPutString(symbols[ival(t)]);
    }

static void
ppParenthesizedExpr(int t)
    {
    ppPutString("(");
    pp(car(t));
    ppPutString(")");
    }

static void
ppArray(int t)
    {
    //if (ppFlags & ppEXPAND)
        //{
        //ppFlags ^= ppEXPAND;
        ppPutString("[");
        ppCSV(t);
        ppPutString("]");
        //ppFlags ^= ppEXPAND;
        //}
    //else
        //{
        //ppPutString("<ARRAY ");
        //ppPutInt(t);
        //ppPutString(">");
        //}
    }

static void
ppJoin(int t)
    {
    //if (ppFlags & ppEXPAND)
        //{
        //ppFlags ^= ppEXPAND;
        ppPutString("(");
        while (type(t) == JOIN)
            {
            pp(car(t));
            t = cdr(t);
            if (type(t) == JOIN)
               ppPutString(",");
            }
        if (t != 0)
           {
           ppPutString(" # ");
           pp(t);
           }

        ppPutString(")");
        //ppFlags ^= ppEXPAND;
        //}
    //else
        //{
        //ppPutString("<LIST ");
        //ppPutInt(t);
        //ppPutString(">");
        //}
    }

static void
ppClosure(int t)
    {
    if (ppFlags & ppEXPAND)
        {
        ppIndent = 0;
        ppPutString("function ");
        if (closure_name(t) != 0)
            pp(closure_name(t));
        ppPutString("(");
        if (closure_parameters(t)) ppCSV(closure_parameters(t));
        ppPutString(")");

        ppPutString("\n");
        ppIndent += 4;
        pp(closure_body(t));
        ppIndent -= 4;
        }
    else
        {
        ppPutString("<function ");
        if (closure_name(t) != 0)
            pp(closure_name(t));
        if (closure_parameters(t))
            pp(closure_parameters(t));
        else
            ppPutString("()");
        ppPutString(">");
        }
    }

static void
ppBlock(int b)
    {
    ppPrintIndent();
    ppPutString("{");
    ppPutString("\n");

    if (car(b) != 0) // non-empty block!
        {
        //printf("//body\n");
        ppSeq(car(b));
        }
    //else
        //printf("//empty body\n");

    ppPrintIndent();
    ppPutString("}");
    if ((ppFlags & ppFLAT) == 0) 
        ppPutString("\n");
    }

static void
ppVarDefList(int  s)
    {
    ppPutString("var ");
    while (s != 0)
        {
        pp(car(s));
        s = cdr(s);
        if (s != 0)
            ppPutString(",");
        }

    ppPutString(";\n");
    }

static void
ppVarDef(int def)
    {
    ppPutString("var ");
    pp(def);
    ppPutString(";");
    }

static void
ppPrintIndent()
    {
    int i;
    for (i = 0; i < ppIndent; ++i)
        ppPutChar(' ');
    }

static void
ppStatement(int  s)
    {
    ppPrintIndent();
    pp(car(s));
    if (type(car(s)) != XCALL)
        {
        ppPutString(";");
        ppPutString("\n");
        }
    }

void
ppSeq(int  s)
    {
    int  statement;

    while (s != 0)
        {
        statement = car(s);
        if (type(statement) != STATEMENT)
            ppPrintIndent();
        pp(statement);
        s = cdr(s);
        }
    }

static void
ppCall(int c)
    {
    int op,args;
    int oldFlags;

    op = car(c);
    args = cdr(c);

    if (isIdentifier(op,"return"))
        {
        pp(op);
        ppPutString(" ");
        ppCSV(args);
        }
    else if (isIdentifier(op,"var"))
        {
        pp(car(args));
        if (cdr(args) != 0 && !isSymbol(cadr(args),UNINITIALIZED))
            {
            ppPutString(" = ");
            pp(cadr(args));
            }
        }
    else if (isIdentifier(op,"lambda"))
        {
        if (ppFlags & ppOBJECT)
            ppEnv(c);
        else 
            {
            ppPutString("function (");
            if (car(args)) ppCSV(car(args));
            ppPutString(") ");
            oldFlags = ppFlags;
            ppFlags |= ppFLAT;
            if ((oldFlags & ppFLAT) == 0) ppFlatLimit = 10000;
            pp(cadr(args));
            ppFlags = oldFlags;
            }
        }
    else if (isIdentifier(op,"function"))
        {
        ppPutString("function ");
        pp(car(args));
        ppPutString("(");
        if (cadr(args)) ppCSV(cadr(args));
        ppPutString(")");
        oldFlags = ppFlags;
        ppFlags |= ppQUOTES;

        ppPutString("\n");
        ppIndent += 4;
        pp(caddr(args));
        ppIndent -= 4;

        ppFlags = oldFlags;
        }
    else
        {
        pp(op);
        if (type(c) == XCALL )
            ppPutString(" ");
        ppPutString("(");
        if (type(c) == CALL)
            {
            ppCSV(args);
            ppPutString(")");
            }
        else
            {
            ppXArgs(args);
            }
        }
    }

void
ppCSV(int  args)
    {
    int i = 0;
    while (args != 0)
        {
        if (car(args) != 0 && type(car(args)) == BLOCK)
            {
            ppPutString("\n");
            ppIndent += 4;
            }
        if (car(args) == 0)
            ppPutString(NULLPTR);
        else
            pp(car(args));
        if (car(args) != 0 && type(car(args)) == BLOCK)
            ppIndent -= 4;
        args = cdr(args);
        if (args != 0)
            ppPutString(",");
        ++i;
        }
    }

static int
blockish(int item)
    {
    return type(item) == BLOCK || type(item) == NAKED_BLOCK;
    }

void
ppXArgs(int  args)
    {
    int firstTime = 1;

    while (args != 0)
        {
        if (cdr(args) != 0        //one extended argument
        &&  cddr(args) == 0
        &&  blockish(car(args)))
            {
            ppPutString(")");
            ppPutString("\n");
            ppIndent += 4;
            pp(car(args));
            ppIndent -= 4;
            ppPrintIndent();
            if (ppFlags & ppFLAT) ppPutString(" ");
            ppPutString("else");

            if (blockish(cadr(args)))
                {
                ppPutString("\n");
                ppIndent += 4;
                pp(cadr(args));
                ppIndent -= 4;
                }
            else
                {
                ppPutString(" ");
                pp(cadr(args));
                }
            args = cdr(args);
            }
        else if (cdr(args) == 0 && blockish(car(args)))
            {
            ppPutString(")"); 
            ppPutString("\n");
            ppIndent += 4;
            pp(car(args));
            ppIndent -= 4;
            }
        else if (firstTime)
            {
            firstTime = 0;
            pp(car(args));
            }
        else
            {
            ppPutString( ", ");
            pp(car(args));
            }

        args = cdr(args);
        }
    }

static void
ppBinary(int b)
    {
    int left,op,right;

    op = car(b);
    left = cadr(b);
    right = caddr(b);

    if (isIdentifier(op,OPEN_BRACKET))
        {
        pp(left);
        ppPutString("[");
        pp(right);
        ppPutString("]");
        }
    else
        {
        pp(left);
        ppPutString(" ");
        pp(op);
        ppPutString(" ");
        pp(right);
        }
    }

void
ppEnv(int obj)
    {
    if (type(obj) == CLOSURE)
        {
        int old = ppFlags;
        ppFlags &= ~ppEXPAND;
        ppClosure(obj);
        ppFlags = old;
        }
    else
        {
        ppPutString("<");
        ppPutString(type(obj));
        ppPutString(" ");
        ppPutInt(obj);
        ppPutString(">");
        }

    if (ppFlags & ppEXPAND)
        {
        int vars,vals;
        int oldFlags;

        oldFlags = ppFlags;

        ppFlags &= ~ppEXPAND;

        ppPutString(":\n");

        vars = car(obj);
        vals = cadr(obj);
        while (vars != 0)
            {
            ppPutString("    ");
            pp(car(vars));
            ppPutString(": ");

            ppFlags |= ppQUOTES;

            ppFlags |= ppFLAT;
            ppFlatLimit = 50;
            ppLength = 0;

            pp(car(vals));

            ppFlags &= ~ppFLAT;
            ppFlags &= ~ppQUOTES;

            ppPutString("\n");

            vars = cdr(vars);
            vals = cdr(vals);
            }
        ppFlags = oldFlags;
        }
    }

void
ppToString(int expr,char *buffer,int size)
    {
    ppPutChar = putCharInString;
    ppPutString = putStringInString;
    ppPutInt = putIntInString;
    ppPutReal = putRealInString;
    ppBuffer = buffer;
    ppBufferSize = size;
    ppBufferIndex = 0;
    ppLastChar = 0;
    pp(expr);
    }

void
ppToFile(int expr,FILE *fp)
    {
    ppPutChar = putCharInFile;
    ppPutString = putStringInFile;
    ppPutInt = putIntInFile;
    ppPutReal = putRealInFile;
    ppOutput = fp;
    ppLastChar = 0;
    ppLength = 0;
    pp(expr);
    }

void
ppStringToFile(char *s,FILE *fp)
    {
    ppOutput = fp;
    putStringInFile(s);
    }

static void
putCharInString(int ch)
    {
    //printf("putting <%c>\n",ch);
    if (ppBufferIndex >= ppBufferSize - 1) return;

    if ((ch == '\n' || ch == '\t' || ch == ' ') && (ppFlags & ppFLAT))
        {
        if (ppLastChar !=  '\n' && ppLastChar != '\t' && ppLastChar != ' ')
            {
            ppBuffer[ppBufferIndex] = ' ';
            ppBuffer[ppBufferIndex+1] = '\0';
            ++ppBufferIndex;
            }
        ppLastChar = ch;
        return;
        }

    ppBuffer[ppBufferIndex] = ch;
        
    ppBuffer[ppBufferIndex+1] = '\0';

    ppLastChar = ch;
    ++ppBufferIndex;
    }

static void
putStringInString(char *s)
    {
    if (ppBufferIndex >= ppBufferSize - 1 - strlen(s)) return;

    while (*s != 0)
        {
        putCharInString(*s);
        ++s;
        }
    }

static void
putIntInString(int i)
    {
    char buffer[64];
    sprintf(buffer,"%d",i);
    putStringInString(buffer);
    }

static void
putRealInString(double r)
    {
    char buffer[64];
    putRealInBuffer(r,buffer);
    putStringInString(buffer);
    }

static void
putRealInBuffer(double r,char *s)
    {
    if (fabs(r) < 10e-6)
        sprintf(s,"%6.6e",r);
    else if (fabs(r) > 10e+6)
        sprintf(s,"%6.6e",r);
    else if (fabs(r) > 1)
        sprintf(s,"%11.*f",(int)(11 - log(fabs(r)) / log(10)),r);
    else
        sprintf(s,"%10.10f",r);
    }

static void
putCharInFile(int ch)
    {
    int outch;
    if ((ch == '\n' || ch == '\t' || ch == ' ') && (ppFlags & ppFLAT))
        {
        if (ppLastChar !=  '\n' && ppLastChar != '\t' && ppLastChar != ' ')
            outch = ' ';
        else
            return;
        }
    else
        outch = ch;

    if ((ppFlags & ppFLAT) && ppLength == ppFlatLimit - 3)
        {
        fprintf(ppOutput,"...");
        ppLength += 3;
        }
    else if (!(ppFlags & ppFLAT) || ppLength < ppFlatLimit)
        {
        fprintf(ppOutput,"%c",outch);
        ++ppLength;
        }
        
    ppLastChar = ch;
    }

static void
putStringInFile(char *s)
    {
    while (*s != 0)
        {
        putCharInFile(*s);
        ++s;
        }
    }

static void
putIntInFile(int i)
    {
    char s[64];
    sprintf(s,"%d",i);
    putStringInFile(s);
    }

static void
putRealInFile(double d)
    {
    char s[64];
    putRealInBuffer(d,s);
    putStringInFile(s);
    }

void
ppStack(char *filename)
    {
    int i;
    int spot;
    FILE *fp;

    printf("saveCount is %d\n",saveCount);
    fp = fopen(filename,"w");
    ppFlags |= ppQUOTES;
    ppFlags |= ppEXPAND;
    fprintf(fp,"Stack:\n[\n");
    for (i = 0; i < saveCount; ++i)
        {
        spot = savedRegisters[i];
#if CHECK_SAVES
        fprintf(fp,"[%d %s] ",saveCount-i-1,savedRegisterNames[i]);
#else
        fprintf(fp,"[%d] ",saveCount-i-1);
#endif
        if (spot == 0)
            fprintf(fp,":null");
        else
            ppToFile(spot,fp);
        if (i < saveCount - 1)
            fprintf(fp,",\n");
        }
    fprintf(fp,"]\n");
    ppFlags &= ~ppQUOTES;
    ppFlags &= ~ppEXPAND;
    fclose(fp);
    }
*/
