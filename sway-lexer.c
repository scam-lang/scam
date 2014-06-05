
/*
 *  Main Author : John C. Lusth
 *  Added Header : Jeffrey Robinson
 *  Last Edit : May 4, 2014
 *
 *  the Sway lexical analyzer 
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

#include "lexer.h"
#include "cell.h"
#include "types.h"
#include "env.h"
#include "util.h"

static int swaySkipWhiteSpace(PARSER *);

char *symbolStop;

int
swayLex(PARSER *p) 
    { 
    int ch; 

    symbolStop = "(){}[];,'";
    ch = swaySkipWhiteSpace(p); 
    //printf("sway lex character is %c\n",ch);

    if (ch == EOF || strchr(symbolStop,ch) != 0) /* single character tokens */ 
        {
        int result = 0;

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
                result = OpenBracketSymbol;
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
        return LexSymbol(p,ch);
        }
    else 
        {
        return throw(LexicalExceptionSymbol,
            "file %s,line %d: unexpected character (%d)\n",
            SymbolTable[p->file],p->line,ch);
        }
    } 

static int
swaySkipWhiteSpace(PARSER *p)
    {
    int ch;

    while ((ch = getNextCharacter(p))
    && ch != EOF && (isspace(ch) || ch == '/'))
        {
        //printf("in sway whitespace <%c>\n", ch);
        if (ch == '/')
            {
            ch = getNextCharacter(p);
            //printf("got slash <%c>\n", ch);
            if (ch == '/')
              {
              ch = getNextCharacter(p);
              //printf("got another slash <%c>\n", ch);
              if (ch == '/') /*skip to end of file*/
                {
                while (ch != EOF) //while (!feof(p))
                    {
                    ch = getNextCharacter(p);
                    }
                }
              else /* skip to end of line */
                  {
                  //printf("skipping to end of line");
                  while (ch != EOF && ch != '\n')
                      ch = getNextCharacter(p);
                  unread(ch, p);
                  }
              }
            else if (ch == '*')
              {
              int more = 1;
              while (more)
                  {
                  while ((ch = getNextCharacter(p))
                  && ch != EOF && ch != '*')
                  {
                      continue;
                  }
                  ch = getNextCharacter(p);
                  if (ch == EOF)
                      Fatal("SOURCE CODE ERROR: unterminated comment\n");
                  more = ch != '/';
                  }
              }
            else
                {
                    unread(ch, p);
                    ch = '/';
                    break;
                }
            }
        }

    return ch;
    }
