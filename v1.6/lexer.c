/*
 *  the scam lexical analyzer
 *
 *  written by John C. Lusth
 *
 *  Fine, old-world hand craftsmanship!
 *  (we don't need no stinking lex!)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "cell.h"
#include "types.h"
#include "parser.h"
#include "env.h"
#include "util.h"

#define LINUX 0

#define FILENAMESIZE 256

void unread(int,PARSER *);
int getNextCharacter(PARSER *);

static int skipWhiteSpace(PARSER *);
int lexNumber(PARSER *,int);
int lexSymbol(PARSER *,int);
int lexString(PARSER *);

char *symbolStop;

int
lex(PARSER *p)
    {
    int ch;

    symbolStop = "();,`'";
    ch = skipWhiteSpace(p);
    //printf("whitespace returns %c", ch);
    //getchar();

    if (ch == EOF || strchr(symbolStop,ch) != 0) /* single character tokens */
        {
        int result;

        switch(ch)
            {
            case EOF:
                result = newPunctuation(END_OF_INPUT);
                break;
            case '(':
                result = newPunctuation(OPEN_PARENTHESIS);
                break;
            case ')':
                result = newPunctuation(CLOSE_PARENTHESIS);
                break;
            case '[':
                result = newPunctuation(OPEN_BRACKET);
                break;
            case ']':
                result = newPunctuation(CLOSE_BRACKET);
                break;
            case '{':
                result = newPunctuation(OPEN_BRACE);
                break;
            case '}':
                result = newPunctuation(CLOSE_BRACE);
                break;
            case '\'':
                result = newPunctuation(QUOTE);
                break;
            case '`':
                result = newPunctuation(BACKQUOTE);
                break;
            case ';':
                result = newPunctuation(SEMI);
                break;
            case ',':
                result = newSymbol(",");
                break;
            default:
                Fatal("%s,line %d: "
                    "INTERNAL ERROR: bad single character token\n",
                    SymbolTable[p->file],p->line);
            }
        file(result) = p->file;
        line(result) = p->line;
        return result;
        }
    else if (isdigit(ch) || ch == '-' || ch == '.') 
        return lexNumber(p,ch); 
    else if (ch == '\"') 
        return lexString(p); 
    else if (isprint(ch))
        return lexSymbol(p,ch);
    else 
        {
        assureMemory("lex:unknown",1000,(int *)0);
        return throw(lexicalExceptionSymbol,
            "file %s,line %d: unexpected character (%d)\n",
            SymbolTable[p->file],p->line,ch);
        }
    } 


int
lexNumber(PARSER *p,int ch)
    {
    char s[512] = "";
    int count;
    int first;
    int decimal,floater,exponent;
    int result;
    int digits;

    first = 1;
    decimal = 0;
    floater = 0;
    exponent = 0;
    digits = 0;

    count = 0;
    while (isdigit(ch) || (first && ch == '-') || (!decimal && ch == '.')
        || (!exponent && (ch == 'E' || ch == 'e')))
        {
        first = 0;
        if (ch == '.')
            {
            decimal = 1;
            floater = 1;
            }
        if (ch == 'E' || ch == 'e')
            {
            decimal = 1;
            floater = 1;
            exponent = 1;
            first = 1;
            }
        if (isdigit(ch)) digits = 1;
        s[count++] = ch;
        if (count >= sizeof(s) - 1)
            {
            assureMemory("lexNumber:size",1000,(int *)0);
            return throw(lexicalExceptionSymbol,
                "file %s,line %d: number has too many digits",
                SymbolTable[p->file],p->line);
            }
        ch = getNextCharacter(p);
        }

    s[count] = '\0';

    if (digits && strchr(" \t\n)];,",ch) == 0)
        {
        assureMemory("lexNumber:illegal",1000,(int *)0);
        return throw(lexicalExceptionSymbol,
            "file %s,line %d: misformed number (%s%c)",
            SymbolTable[p->file],p->line,s,ch);
        }

    unread(ch,p);

    //printf("number is %s\n", s);

    if (strcmp(s,"-") == 0)
        result = lexSymbol(p,s[0]);
    else if (strcmp(s,".") == 0)
        result = lexSymbol(p,s[0]);
    else if (floater)
        {
        result = newReal(atof(s));
        }
    else
        {
        result = newInteger(atoi(s));
        }

    file(result) = p->file;
    line(result) = p->line;

    return result;
    }

int
lexSymbol(PARSER *p,int ch)
    {
    int index;
    char buffer[512];
    int result;

    buffer[0] = ch;
    index = 1;

    while ((ch = getNextCharacter(p)) && ch != EOF
    && !isspace(ch) && strchr(symbolStop,ch) == 0)
        {
        //printf("symbol: %c\n", ch);
            buffer[index++] = ch;
        if (index == sizeof(buffer))
            {
            assureMemory("lexSymbol:size",1000,(int *)0);
            return throw(lexicalExceptionSymbol,
                "file %s,line %d: token has too many characters",
                SymbolTable[p->file],p->line);
            }
        }

    unread(ch,p);

    buffer[index] = '\0';
    //printf("lexSymbol: buffer is %s\n", buffer);

    result = newSymbol(buffer);

    file(result) = p->file;
    line(result) = p->line;

    return result;
    }

int
lexString(PARSER *p)
    {
    int ch;
    int index;
    char buffer[4096];
    int result;
    int lineNumber;

    index = 0;

    lineNumber = p->line;
    while ((ch = getNextCharacter(p)) && ch != EOF)
        {
        if (ch == '\"')
            break;

        if (ch == '\\')
            {
            ch = getNextCharacter(p);
            if (ch == EOF)
                {
                assureMemory("lexString:eof",1000,(int *) 0);
                return throw(lexicalExceptionSymbol,
                    "file %s,line %d: "
                    "unexpected end of file (last char was a backslash)\n",
                    SymbolTable[p->file],p->line);
                }
            if (ch == 'n')
                buffer[index++] = '\n';
            else if (ch == 't')
                buffer[index++] = '\t';
            else if (ch == 'r')
                buffer[index++] = '\r';
            else
                buffer[index++] = ch;
            }
        else if (ch == '\n')
            buffer[index++] = '\n';
        else
            buffer[index++] = ch;

        if (index == sizeof(buffer) - 1)
            {
            assureMemory("lexString:size",1000,(int *) 0);
            return throw(lexicalExceptionSymbol,
                "file %s,line %d: string has too many characters",
                SymbolTable[p->file],lineNumber);
            }
        }

    buffer[index] = '\0';

    if (ch != '\"')
        {
        assureMemory("lexString:unterminated",1000,(int *) 0);
        return throw(lexicalExceptionSymbol,
            "file %s,line %d: unterminated string",
            SymbolTable[p->file],lineNumber);
        }

    /* nil is the empty string */

    if (buffer[0] == '\0') return 0;

    result = newString(buffer);
    //printf("line %d: indent of new string \"%s\" is %d\n",
        //line(result),buffer,indent(result));

    file(result) = p->file;
    line(result) = p->line;

    return result;
    }

int
getNextCharacter(PARSER *p)
    {
    int ch;

    if (p->pushedBack)
        {
        p->pushedBack = 0;
        //printf("getNextCharacter: returning pushed back <%c>\n",p->pushBack);
        if (p->pushBack == '\n') ++(p->line);
        return p->pushBack;
        }
    else
        {
        ch = fgetc(p->input);
        //printf("getNextCharacter: returning <%c> %d\n",ch, ch);
        if (ch == '\n') ++(p->line);
        return ch;
        }
    }

void
unread(int ch,PARSER *p)
    {
    if (ch == '\n') --(p->line);
    p->pushedBack = 1;
    p->pushBack = ch;
    }

static int
skipWhiteSpace(PARSER *p)
    {
    int ch;

    while ((ch = getNextCharacter(p))
    && ch != EOF && (isspace(ch) || ch == ';'))
        {
        if (ch == ';')
            {
            ch = getNextCharacter(p);
            if (ch == '$') /* skip to end of file */
                {
                while ((ch = getNextCharacter(p)) && ch != EOF)
                    continue;
                }
            else if (ch == '{') /* skip to close comment */
                {
                int prev = ch;
                int lineNumber = p->line;
                while ((ch = getNextCharacter(p))
                && ch != EOF && (prev != ';' || ch != '}'))
                    {
                    prev = ch;
                    }
                if (ch == EOF)
                    {
                    return Fatal("file %s,line %d: unterminated comment\n",
                            SymbolTable[p->file],lineNumber);
                    }
                }
            else /* skip to end of line */
                {
                while (ch != EOF && ch != '\n')
                    ch = getNextCharacter(p);
                }
            }
        }

    return ch;
    }
