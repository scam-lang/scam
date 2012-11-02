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
    && ch != EOF && (isspace(ch) || ch == '/'))
        {
        if (ch == '/')
            {
            ch = getNextCharacter(p);
            if (ch == '/')
            {
            ch = getNextCharacter(p);
            if (ch == '/') /*skip to end of file*/
              {
              ch = getNextCharacter(p);
              while (ch != EOF) //while (!feof(p))
              {
              ch = getNextCharacter(p);
              }
              }
            }
            else /* skip to end of line */
                {
                while (ch != EOF && ch != '\n')
                ch = getNextCharacter(p);
                unread(ch);
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
                    /* Fatal("SOURCE CODE ERROR\n"
                        "file %s, line %d: unterminated comment\n",
                        cellString(0,0,fn), ln); */
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

    return ch;
    }
