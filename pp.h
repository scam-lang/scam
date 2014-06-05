
/*
 *  Main Author : John C. Lusth
 *  Added Header : Jeffrey Robinson
 *  Last Edit : May 4, 2014
 *
 *  The layout of this document is such:
 *      extern variables
 *      external functions
 */

#ifndef __PP_H__
#define __PP_H__

extern int ppQuoting;

extern void scamPP(FILE *,int);
extern void swayPP(FILE *,int);
extern void scamPP(FILE *,int);
extern void swayPPLevel(FILE *,int,int);
extern void ppObject(FILE *,int,int);
extern void ppList(FILE *,char *,int,char *,int);
extern void ppArray(FILE *,char *,int,char *,int);
extern void ppCons(FILE *,int,int);
extern void ppTable(FILE *,int,int);
extern void ppFunction(FILE *,int,int);
extern void ppLevel(FILE *,int,int);
extern void debug(char *,int);
extern void debugOut(FILE *, char *, int);
extern void ppToString(int,char *,int,int *);

#endif
