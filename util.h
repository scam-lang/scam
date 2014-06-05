
/*
 *  Main Author : John C. Lusth
 *  Barely Authors : Jeffrey Robinson, Gabriel Loewen
 *  Last Edit : May 4, 2014
 *
 *  The layout of this document is such:
 *      Includes
 *      external functions
 */


#ifndef __UTIL_H__
#define __UTIL_H__

/* FILE */
#include <stdio.h>

extern int ProcessOptions(int,char **);     /* command line option handler */
extern int Fatal(char *,...);               /* Output error and exit */

#endif
