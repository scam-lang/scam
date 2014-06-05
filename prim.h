
/*
 *  Main Author : John C. Lusth
 *  Added Header : Jeffrey Robinson
 *  Last Edit : May 4, 2014
 *
 *  The layout of this document is such:
 *      typedefs
 *      external functions
 */

#ifndef __PRIM_H__
#define __PRIM_H__

typedef int (*PRIM)(int);
extern PRIM BuiltIns[];
extern void loadBuiltIns(int);
extern void installArgsEnv(int,int,char **,char **,int);

#endif
