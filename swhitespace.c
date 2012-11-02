#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "cell.h"
#include "parser.h"

extern int getNextCharacter(PARSER * p);
extern int Fatal(char* ,...);
extern void unread(int, PARSER *);

int
skipWhiteSpace(PARSER *p)
    {
    int ch;

    while ((ch = getNextCharacter(p))
    && ch != EOF && (isspace(ch) || ch == '/'))
        {
        printf("in whitespace <%c>\n", ch);
        if (ch == '/')
            {
            ch = getNextCharacter(p);
            printf("got slash <%c>\n", ch);
            if (ch == '/')
              {
              ch = getNextCharacter(p);
              printf("got another slash <%c>\n", ch);
              if (ch == '/') /*skip to end of file*/
                {
                while (ch != EOF) //while (!feof(p))
                    {
                    ch = getNextCharacter(p);
                    }
                }
              else /* skip to end of line */
                  {
                  printf("skipping to end of line");
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
                      /* Fatal("SOURCE CODE ERROR\n"
                          "file %s, line %d: unterminated comment\n",
                          cellString(0,0,fn), ln); */
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
