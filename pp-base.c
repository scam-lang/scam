#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "pp-base.h"

int ppIndent = 0;
int ppFlatLimit = 65;
int ppLength = 0;
int ppFlags = 0;

FILE *ppOutput;

static void putCharToString(int);
static void putStringToString(char *);
static void putIntToString(int);
static void putRealToString(double);

static void putCharToFile(int);
static void putStringToFile(char *);
static void putIntToFile(int);
static void putRealToFile(double);

void (*ppPutChar)(int) = putCharToFile;
void (*ppPutString)(char *) = putStringToFile;
void (*ppPutInt)(int) = putIntToFile;
void (*ppPutReal)(double) = putRealToFile;

static char *ppBuffer;
static int  ppBufferIndex;
static int  ppBufferSize;
static int  ppLastChar = 0;

void
ppPrintIndent()
    {
    int i;
    for (i = 0; i < ppIndent; ++i)
        ppPutChar(' ');
    }

void
ppToString(char *buffer,int size)
    {
    ppPutChar = putCharToString;
    ppPutString = putStringToString;
    ppPutInt = putIntToString;
    ppPutReal = putRealToString;
    ppBuffer = buffer;
    ppBufferSize = size;
    ppBufferIndex = 0;
    ppLastChar = 0;
    }

void
ppToFile(FILE *fp)
    {
    ppPutChar = putCharToFile;
    ppPutString = putStringToFile;
    ppPutInt = putIntToFile;
    ppPutReal = putRealToFile;
    ppOutput = fp;
    ppLastChar = 0;
    ppLength = 0;
    }

static void
putCharToString(int ch)
    {
    //printf("putting <%c>\n",ch);
    if (ppBufferIndex >= ppBufferSize - 1) return;

    if ((ch == '\n' || ch == '\t' || ch == ' ') && (ppFlags & ppFLAT))
        {
        if (ppLastChar !=  '\n' && ppLastChar != '\t' && ppLastChar != ' ')
            {
            ppBuffer[ppBufferIndex] = ' ';
            ppBuffer[ppBufferIndex+1] = '\0';
            ++ppBufferIndex;
            }
        ppLastChar = ch;
        return;
        }

    ppBuffer[ppBufferIndex] = ch;
        
    ppBuffer[ppBufferIndex+1] = '\0';

    ppLastChar = ch;
    ++ppBufferIndex;
    }

static void
putStringToString(char *s)
    {
    if (ppBufferIndex >= ppBufferSize - 1 - strlen(s)) return;

    while (*s != 0)
        {
        putCharToString(*s);
        ++s;
        }
    }

static void
putIntToString(int i)
    {
    char buffer[64];
    sprintf(buffer,"%d",i);
    putStringToString(buffer);
    }

static void
putRealToString(double r)
    {
    char s[64];
    if (fabs(r) < 10e-6)
        sprintf(s,"%6.6e",r);
    else if (fabs(r) > 10e+6)
        sprintf(s,"%6.6e",r);
    else if (fabs(r) > 1)
        sprintf(s,"%11.*f",(int)(11 - log(fabs(r)) / log(10)),r);
    else
        sprintf(s,"%10.10f",r);
    putStringToString(s);
    }

static void
putCharToFile(int ch)
    {
    int outch;
    if ((ch == '\n' || ch == '\t' || ch == ' ') && (ppFlags & ppFLAT))
        {
        if (ppLastChar !=  '\n' && ppLastChar != '\t' && ppLastChar != ' ')
            outch = ' ';
        else
            return;
        }
    else
        outch = ch;

    if ((ppFlags & ppFLAT) && ppLength == ppFlatLimit - 3)
        {
        fprintf(ppOutput,"...");
        ppLength += 3;
        }
    else if (!(ppFlags & ppFLAT) || ppLength < ppFlatLimit)
        {
        fprintf(ppOutput,"%c",outch);
        ++ppLength;
        }
        
    ppLastChar = ch;
    }

static void
putStringToFile(char *s)
    {
    while (*s != 0)
        {
        putCharToFile(*s);
        ++s;
        }
    }

static void
putIntToFile(int i)
    {
    char s[64];
    sprintf(s,"%d",i);
    putStringToFile(s);
    }

static void
putRealToFile(double r)
    {
    char s[64];
    if (fabs(r) < 10e-6)
        sprintf(s,"%6.6e",r);
    else if (fabs(r) > 10e+6)
        sprintf(s,"%6.6e",r);
    else if (fabs(r) > 1)
        sprintf(s,"%11.*f",(int)(11 - log(fabs(r)) / log(10)),r);
    else
        sprintf(s,"%10.10f",r);
    putStringToFile(s);
    }
