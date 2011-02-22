/*  
 *  the xcheme lexical analyzer 
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
#include "lexer.h"
#include "util.h"

#define LINUX 0

#define FILENAMESIZE 256

int pushBack;
int pushedBack = 0;

static int skipWhiteSpace(PARSER *);
static int lexNumber(PARSER *);
static int lexSymbol(PARSER *,int);
static int lexString(PARSER *);

int
lex(PARSER *p) 
    { 
    int ch; 

    LineNumber = p->line;
    FileIndex = p->file;

    ch = skipWhiteSpace(p); 

    p->line = LineNumber;

    switch(ch) 
        { 
        /* single character tokens */ 
        case EOF: 
            return newPunctuation(END_OF_INPUT);
        case '(': 
            return newPunctuation(OPEN_PARENTHESIS);
        case ')': 
            return newPunctuation(CLOSE_PARENTHESIS);
        case '\'': 
            return newPunctuation(QUOTE);
        case ',': 
            return newPunctuation(COMMA);
        default: 
            /* numbers, tokens, and strings */ 
            if (isdigit(ch) || ch == '-' || ch == '.') 
                {
                int result;
                unread(ch);
                result = lexNumber(p); 
                return result;
                }
            else if (ch == '\"') 
                return lexString(p); 
            else
                return lexSymbol(p,ch);
        } 
    Fatal("line %d: unexpected character (%c)\n", LineNumber,ch);
    } 

static int
skipWhiteSpace(PARSER *p)
    {
    int ch;

    while ((ch = getNextCharacter(p->input))
    && ch != EOF && (isspace(ch) || ch == ';'))
        {
        if (ch == '\n') ++LineNumber;
        if (ch == ';')
            { 
            ch = getNextCharacter(p->input);
            if (ch == '$') /* skip to end of file */
                {
                while ((ch = getNextCharacter(p->input)) && ch != EOF)
                    {
                    if (ch == '\n') ++LineNumber;
                    }
                }
            else if (ch == '{') /* skip to close comment */
                {
                int prev = ch;
                while ((ch = getNextCharacter(p->input))
                && ch != EOF && (prev != ';' || ch != '}'))
                    {
                    if (ch == '\n') ++LineNumber;
                    prev = ch;
                    }
                if (ch == EOF)
                    Fatal("SOURCE CODE ERROR\n"
                        "line %d: unterminated comment\n", LineNumber);
                }
            else /* skip to end of line */
                {
                while (ch != EOF && ch != '\n')
                    ch = getNextCharacter(p->input);
                }
            }
        }

    return ch;
    }

static int
lexNumber(PARSER *p)
    {
    int ch;
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
    ch = getNextCharacter(p->input);
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
            Fatal("SOURCE CODE ERROR\nline %d\n"
                "number is too large\n",LineNumber);
        ch = getNextCharacter(p->input);
        }

    s[count] = '\0';

    if (digits && strchr(" \t\n()[];,",ch) == 0)
        return newPunctuation(BAD_NUMBER);

    unread(ch);

    //printf("number is %s\n", s);

    if (strcmp(s,"-") == 0)
        result = lexSymbol(p,s[0]);
    else if (strcmp(s,".") == 0)
        result = lexSymbol(p,s[0]);
    else if (floater)
        result = newReal(atof(s));
    else
        result = newInteger(atoi(s));

    return result;
    }

static int
lexSymbol(PARSER *p,int ch)
    {
    int index;
    char buffer[512];
    int result;

    buffer[0] = ch;
    index = 1;

    while ((ch = getNextCharacter(p->input)) && ch != EOF
    && !isspace(ch) && strchr("();,'",ch) == 0)
        {
        //printf("symbol: %c\n", ch);
            buffer[index++] = ch;
        if (index == sizeof(buffer))
            Fatal("SOURCE CODE ERROR\nline %d\n"
                "token too large\n",LineNumber);
        }

    unread(ch);

    buffer[index] = '\0';
    //printf("lexSymbol: buffer is %s\n", buffer);

    result = newSymbol(buffer);

    return result;
    }

static int
lexString(PARSER *p)
    {
    int ch;
    int index;
    char buffer[4096];
    int result;
    int backslashed;

    index = 0;

    backslashed = 0;
    while ((ch = getNextCharacter(p->input)) && ch != EOF)
        {
        if (ch == '\"')
            break;

        if (ch == '\\')
            {
            ch = getNextCharacter(p->input);
            if (ch == EOF)
                Fatal("SOURCE CODE ERROR\nline %d\n"
                    "unexpected end of file (last char was a backslash)\n",
                    LineNumber);
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
            {
            ++LineNumber;
            buffer[index++] = '\n';
            }
        else
            buffer[index++] = ch;

        if (index == sizeof(buffer) - 1)
            Fatal("SOURCE CODE ERROR\nline %d\n"
                "string too large\n",LineNumber);
        }

    buffer[index] = '\0';

    if (ch != '\"')
        {
        Fatal("SOURCE CODE ERROR\nline %d\nunterminated string: \"",
            LineNumber);
        }

    //printf("string is <%s>\n", buffer);

    result = newString(buffer);
    //printf("line %d: indent of new string \"%s\" is %d\n",
        //line(result),buffer,indent(result));

    p->line = LineNumber;
    return result;
    }

int
getNextCharacter(FILE *fp)
    {
    int ch;

    if (pushedBack)
        {
        pushedBack = 0;
        //printf("getNextCharacter: returning pushed back <%c>\n",pushBack);
        return pushBack;
        }
    else
        {
        ch = fgetc(fp);
        //printf("getNextCharacter: returning <%c>\n",ch);
        return ch;
        }
    }

void
unread(int ch)
    {
    pushedBack = 1;
    pushBack = ch;
    }
