
/*
 *  Main Author : John C. Lusth
 *  Added Header : Jeffrey Robinson
 *  Last Edit : May 4, 2014
 *
 *  The layout of this document is such:
 *      punctuation, keywords, types, lexer stuff.
 */

#ifndef __TYPES_H__
#define __TYPES_H__

/* scam punctuation */

extern char *CLOSE_PARENTHESIS;// = "CLOSE_PARENTHESIS";
extern char *OPEN_PARENTHESIS;// = "OPEN_PARENTHESIS";
extern char *QUOTE;//= "QUOTE";
extern char *BACKQUOTE;//= "QUOTE";
extern char *COMMA;// = "COMMA";
extern char *SEMICOLON;// = "SEMICOLON";
extern char *END_OF_INPUT;// = "END_OF_INPUT";

/* scam keywords */

extern char *BAD_NUMBER;// = "BAD_NUMBER";

/* scam internal types */

extern char *INTEGER;// = "INTEGER";
extern char *REAL;// = "REAL";
extern char *STRING;// = "STRING";
extern char *SYMBOL;// = "SYMBOL";
extern char *CONS;// = "THUNK";
extern char *ARRAY;// = "ARRAY";
extern char *UNINITIALIZED;// = "UNINITIALIZED";

/* scam types for an imperative front-end */

extern char *OPEN_BRACE;//= "OPEN_BRACE";
extern char *CLOSE_BRACE;//= "CLOSE_BRACE";
extern char *OPEN_BRACKET;//= "OPEN_BRACKET";
extern char *CLOSE_BRACKET;//= "CLOSE_BRACKET";
extern char *SEMI;//= "SEMICOLON";

#endif
