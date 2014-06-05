
/*
 *  Main Author : Jeffrey Robinson
 *  Barely Authors : John C. Lusth, Gabriel Loewen
 *  Last Edit : May 4, 2014
 *
 *  The layout of this document is such:
 *      Includes
 *      external functions
 */

#ifndef __LEXER_H__
#define __LEXER_H__

#include "parser.h"

extern int lex(PARSER *p);

extern void unread(int,PARSER *);
extern int getNextCharacter(PARSER *);

extern int lexNumber(PARSER *,int);
extern int LexSymbol(PARSER *,int);
extern int lexString(PARSER *);

extern int swayLex(PARSER *);

#endif
