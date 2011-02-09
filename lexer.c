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
#include "lexer.h"
#include "util.h"

#define LINUX 0

#define FILENAMESIZE 256

FILE *Input;

int pushBack;
int pushedBack = 0;

static int skipWhiteSpace(void);
static int lexNumber(void);
static int lexSymbol(int);
static int lexString(void);

int
lex() 
    { 
    int ch; 

    ch = skipWhiteSpace(); 

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
                result = lexNumber(); 
                return result;
                }
            else if (ch == '\"') 
                return lexString(); 
            else
                return lexSymbol(ch);
        } 
    Fatal("line %d: unexpected character (%c)\n", LineNumber,ch);
    } 

static int
skipWhiteSpace()
    {
    int ch;

    while ((ch = getNextCharacter(Input))
    && ch != EOF && (isspace(ch) || ch == '/'))
        {
        if (ch == '\n')
            {
            ++LineNumber;
            }
        else if (ch == ';')
            { 
            ch = getNextCharacter(Input);
            if (ch == ';')
            {
            ch = getNextCharacter(Input);
            if (ch == ';') /* skip to end of file */
                {
                ch = getNextCharacter(Input);
                while (!feof(Input))
                {
                if (ch == '\n')
                    {
                    ++LineNumber;
                    }
                    ch = getNextCharacter(Input);
                }
                }
            else /* skip to end of line */
                {
                while (ch != EOF && ch != '\n')
                ch = getNextCharacter(Input);
                unread(ch);
                }
            }
            else if (ch == '*')
            {
            int more = 1;
            while (more)
                {
                while ((ch = getNextCharacter(Input))
                && ch != EOF && ch != '*')
                {
                if (ch == '\n')
                    {
                    ++LineNumber;
                    }
                }
                ch = getNextCharacter(Input);
                if (ch == '\n')
                    {
                    ++LineNumber;
                    }
                if (ch == EOF)
                    Fatal("SOURCE CODE ERROR\n"
                        "line %d: unterminated comment\n",
                        LineNumber);
                more = ch != '/';
                }
            }
            else
            {
                unread(ch);
            ch = '/';
            break;
            }
            }
        }

    return ch;
    }

static int
lexNumber()
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
    ch = getNextCharacter(Input);
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
        ch = getNextCharacter(Input);
        }

    s[count] = '\0';

    if (digits && strchr(" \t\n()[];,",ch) == 0)
        return newPunctuation(BAD_NUMBER);

    unread(ch);

    //printf("number is %s\n", s);

    if (strcmp(s,"-") == 0)
        result = lexSymbol(s[0]);
    else if (strcmp(s,".") == 0)
        result = lexSymbol(s[0]);
    else if (floater)
        result = newReal(atof(s));
    else
        result = newInteger(atoi(s));

    return result;
    }

static int
lexSymbol(int ch)
    {
    int index;
    char buffer[512];
    int result;

    buffer[0] = ch;
    index = 1;

    while ((ch = getNextCharacter(Input)) && ch != EOF
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
lexString()
    {
    int ch;
    int index;
    char buffer[4096];
    int result;
    int backslashed;

    index = 0;

    backslashed = 0;
    while ((ch = getNextCharacter(Input)) && ch != EOF)
        {
        if (ch == '\"')
            break;

        if (ch == '\\')
            {
            ch = getNextCharacter(Input);
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
