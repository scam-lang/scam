#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "cell.h"
#include "parser.h"

extern int getNextCharacter(PARSER * p);
extern int Fatal(char* ,...);

int
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
