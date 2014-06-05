
/*
 *  Main Author : John C. Lusth
 *  Added Header : Jeffrey Robinson
 *  Last Modified : May 4, 2014
 *
 *  TODO : Description
 *
 */

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include "cell.h"
#include "parser.h"
#include "util.h"

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
