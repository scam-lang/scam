
/*
 *  Main Author : Jeffrey Robinson
 *  Barely Authors : John C. Lusth, Gabriel Loewen
 *  Last Edit : May 4, 2014
 *
 *  The layout of this document is such:
 *      Includes
 *      Structs
 *      external functions
 *      #defines
 */


#ifndef __PARSER_H__
#define __PARSER_H__

/* FILE */
#include <stdio.h>

typedef struct parsertag
    {
    char *fileName;
    int pending;
    int file;
    int line;
    int pushBack;
    int pushedBack;
    FILE *input;
    FILE *output;
    char *buffer;
    int bufferIndex;
    } PARSER;

extern int scamParse(PARSER *);
extern int swayParse(PARSER *);
extern PARSER *newParser(char *);
extern PARSER *newParserFP(FILE *,char *);
extern void freeParser(PARSER *);
extern void ppf(char*,int,char*);
extern int exprSeq(PARSER *);
extern int expr(PARSER *);

#define SCAM 1
#define SWAY 2


#endif
