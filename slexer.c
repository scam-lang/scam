/*  
 *  the Sway lexical analyzer 
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
#include "env.h"
#include "util.h"


#define LINUX 0

#define FILENAMESIZE 256


char savedCharacters[512];
int saveIndex = 0;
int lastSaved = 0;

FILE *Input;
int (*getNextCharacter)(FILE *) = fileGetChar;
char *prompt = "sway> ";

int pushBack;
int pushedBack = 0;

static char *inputBuffer = 0;
static int inputBufferIndex = 0;

static int skipWhiteSpace(void);
static int lexNumber(void);
static int lexSymbol(int);
static int lexString(void);

static void addToSave(int);

int
lex() 
    { 
    int ch; 

    ch = skipWhiteSpace(); 

    switch(ch) 
        { 
        /* single character tokens */ 
        case EOF: 
            return cons(END_OF_INPUT,0,0);
        case '(': 
            return cons(OPEN_PARENTHESIS,0,0);
        case ')': 
            return cons(CLOSE_PARENTHESIS,0,0);
        case '{': 
            return cons(OPEN_BRACE,0,0);
        case '}': 
            return cons(CLOSE_BRACE,0,0);
        case '[': 
            return newIdentifier("[");
        case ']': 
            return cons(CLOSE_BRACKET,0,0);
        case ',': 
            return cons(COMMA,0,0);
        case ';': 
            return cons(SEMI,0,0);
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
    Fatal("unexpected character (%c)\n", ch);
    } 

static int
skipWhiteSpace()
    {
    int ch;
    int spot = 0;

    while ((ch = getNextCharacter(Input))
    && ch != EOF && (isspace(ch) || ch == '/'))
        {
        if (ch == '\n')
            {
            ++LineNumber;
            spot = 0;
            Indenting = 1;
            }
        else if (Indenting && ch == ' ') ++spot;
        else if (Indenting && ch == '\t') spot = (spot / 8 + 1) * 8;
        else if (ch == '/')
            { 
            ch = getNextCharacter(Input);
            if (ch == '/')
            {
            ch = getNextCharacter(Input);
            if (ch == '/') /* skip to end of file */
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
            int ln = LineNumber;
            int fn = FileIndex;
            int more = 1;
            spot += 2;
            while (more)
                {
                while ((ch = getNextCharacter(Input))
                && ch != EOF && ch != '*')
                {
                if (ch == '\t') spot = (spot / 8 + 1) * 8;
                else ++spot;
                if (ch == '\n')
                    {
                    ++LineNumber;
                    spot = 0;
                    }
                }
                ++spot;
                ch = getNextCharacter(Input);
                ++spot;
                if (ch == '\n')
                    {
                    ++LineNumber;
                    spot = 0;
                    }
                if (ch == EOF)
                    Fatal("SOURCE CODE ERROR\n"
                        "file %s,line %d: unterminated comment\n",
                cellString(0,0,fn), ln);
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

    //printf("non-whitespace: %c\n",ch);
    if (!Indenting)
        Indent = -1;
    else
        {
        Indenting = 0;
        Indent = spot;
        }
    //printf("final indent is %d\n",Indent);
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
            Fatal("SOURCE CODE ERROR\nfile %s,line %d\n"
                "number is too large\n",symbols[FileIndex],LineNumber);
        ch = getNextCharacter(Input);
        }

    s[count] = '\0';

    if (digits && strchr(" \t\n()[];,",ch) == 0)
        return cons(BAD_NUMBER,0,0);

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
    && !isspace(ch) && strchr("(){}[];,",ch) == 0)
        {
        //printf("symbol: %c\n", ch);
            buffer[index++] = ch;
        if (index == sizeof(buffer))
            Fatal("SOURCE CODE ERROR\nfile %s,line %d\n"
                "token too large\n",symbols[FileIndex],LineNumber);
        }

    unread(ch);

    buffer[index] = '\0';
    //printf("lexSymbol: buffer is %s\n", buffer);

    //if (strcmp(buffer,VAR) == 0)
        //return cons(VAR,0,0);
    //else if (strcmp(buffer,FUNCTION) == 0)
        //return cons(FUNCTION,0,0);
    if (strcmp(buffer,ALTERNATIVE) == 0)
        return cons(ALTERNATIVE,0,0);
    else if (buffer[0] == ':')
        {
        if (strcmp(buffer+1,NULLPTR) == 0)
            result = 0;
        else
            result = newSymbol(buffer+1);
        }
    else
        result = newIdentifier(buffer);

    return result;
    }

static int
lexString()
    {
    int ch;
    int index;
    char buffer[4096];
    int result;
    int ln = LineNumber;
    int fn = FileIndex;
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
                Fatal("SOURCE CODE ERROR\nfile %s,line %d\n"
                    "unexpected end of file (last char was a backslash)\n",
                    cellString(0,0,fn), ln);
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
            Fatal("SOURCE CODE ERROR\nfile %s,line %d\n"
                "string too large\n",cellString(0,0,fn), ln);
        }

    buffer[index] = '\0';

    if (ch != '\"')
        {
        int i;
        printf("SOURCE CODE ERROR\nfile %s,line %d\nunterminated string: \"",
            cellString(0,0,fn), ln);
        for (i = 0; i < 40; ++i)
            {
            if (buffer[i] == '\n') break;
            putchar(buffer[i]);
            }
        Fatal("unterminated string\n");
        }

    //printf("string is <%s>\n", buffer);

    result = newString(buffer);
    //printf("line %d: indent of new string \"%s\" is %d\n",
        //line(result),buffer,indent(result));

    return result;
    }

int
consoleGetChar(FILE *fp)
    {
    char result;

    //printf("getting char from console, file is %p\n",fp);
    if (pushedBack)
        {
        pushedBack = 0;
        //printf("consoleGetChar: returning pushBack <%c>\n",pushBack);
        return pushBack;
        }

    //printf("console: inputBuffer is <%s>\n", inputBuffer);
    //printf("console: inputBufferIndex is %d\n", inputBufferIndex);

    if (inputBufferIndex == 0) 
        {
        inputBuffer = readline(prompt);
        //printf("buffer allocated at %p\n",inputBuffer);
        //printf("buffer allocated at %p\n",inputBuffer);
        //printf("inputBuffer allocated at %p\n",inputBuffer);
        prompt = "more> ";
        }

    if (inputBuffer == 0)
        return EOF;

    if (inputBuffer[inputBufferIndex] == '\0')
        {
        //printf("freeing input buffer...\n");
#if LINUX
        free(inputBuffer);
#endif
        //printf("input buffer freed...\n");
        inputBuffer = 0;
        inputBufferIndex = 0;
        result = '\n';
        }
    else
        result = inputBuffer[inputBufferIndex++];

    //printf("consoleGetChar: returning <%c>\n",result);

    addToSave(result);

    return result;
    }

void
clearline()
    {
    pushedBack = 0;
    if (inputBuffer == 0) return;

    while (inputBuffer[inputBufferIndex++] != '\0')
        addToSave(inputBuffer[inputBufferIndex++]);

#if LINUX
    free(inputBuffer);
#endif

    inputBuffer = 0;
    inputBufferIndex = 0;
    }

static void
addToSave(int ch)
    {
    if (saveIndex < sizeof(savedCharacters) - 1)
        {
        if (isspace(ch))
            {
            if (!isspace(lastSaved) && lastSaved != 0)
                {
                savedCharacters[saveIndex++] = ' ';
                savedCharacters[saveIndex] = '\0';
                }
            }
        else
            {
            savedCharacters[saveIndex++] = ch;
            savedCharacters[saveIndex] = '\0';
            }

        //printf("save now is: %s\n",savedCharacters);
        }

    lastSaved = ch;
    }

int
fileGetChar(FILE *fp)
    {
    int ch;

    if (pushedBack)
        {
        pushedBack = 0;
        //printf("fileGetChar: returning pushed back <%c>\n",pushBack);
        return pushBack;
        }
    else
        {
        ch = fgetc(fp);
        //printf("fileGetChar: returning <%c>\n",ch);
        return ch;
        }
    }

void
unread(int ch)
    {
    pushedBack = 1;
    pushBack = ch;
    }

int
stringGetChar(FILE *fp)
    {
    extern char *InputString;
    extern int InputStringIndex;
    extern int InputStringLength;

    int ch;

    if (pushedBack)
        {
        pushedBack = 0;
        //printf("stringGetChar: returning pushed back <%c>\n",pushBack);
        return pushBack;
        }
    else if (InputStringIndex == InputStringLength)
        {
        return -1;
        }
    else
        {
        ch = InputString[InputStringIndex++];
        //printf("stringGetChar: returning <%c>\n",ch);
        return ch;
        }
    }
