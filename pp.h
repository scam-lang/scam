
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

extern void scamPPFile(FILE *,int);
extern void swayPPFile(FILE *,int);
extern void scamPPString(char *,int,int);
extern void swayPPString(char *,int,int);
extern void ppTable(int,int);
extern void debug(char *,int);
extern void debugOut(FILE *, char *, int);
extern void scamppToString(int,char *,int,int *);
extern void swayppToString(int,char *,int,int *);

extern int ppActual;
extern int ppQuoting;

#endif
