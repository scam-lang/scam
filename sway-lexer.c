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

static int swaySkipWhiteSpace(PARSER *);


int
swaylex(PARSER *p) 
    { 
    int ch; 

    ch = swaySkipWhiteSpace(p); 
    printf("lex character is %c\n",ch);

    if (ch == EOF || strchr("()'`,",ch) != 0) /* single character tokens */ 
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
            case ';': 
                result = newPunctuation(SEMI);
                break;
            case ',': 
                result = newPunctuation(COMMA);
                break;
            default:
                Fatal("INTERNAL ERROR: bad single character token\n");
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
        {
        symbolStop = "(){}[];,'";
        return lexSymbol(p,ch);
        }
    else 
        {
        assureMemory("lex:unknown",1000,(int *)0);
        return throw(lexicalExceptionSymbol,
            "file %s,line %d: unexpected character (%d)\n",
            SymbolTable[p->file],p->line,ch);
        }
    } 

static int
swaySkipWhiteSpace(PARSER *p)
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
