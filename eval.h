
/*
 *  Main Author : John C. Lusth
 *  Barely Authors : Jeffrey Robinson, Gabriel Loewen
 *  Last Edit : May 4, 2014
 *
 *  The layout of this document is such:
 *      external functions
 *      #defines
 */

#ifndef __EVAL_H__
#define __EVAL_H__

extern int eval(int,int);
extern int evalCall(int,int,int);
extern int evalBlock(int,int,int);
extern int makeRun(int (*f)(int,int),int);

#define NORMAL 1
#define NO_EVALUATION 2
#define PASS_THROUGH 3

#define ALL 3
#define ALLBUTLAST 4

#endif
