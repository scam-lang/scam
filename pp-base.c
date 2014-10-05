#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pp-base.h"

#include "scam.h"
#include "cell.h"

int ppIndent = 0;
int ppFlat = 0;
int ppLength = 0;
int ppMaxLength = 0;

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

static void (*oldPutChar)(int) = putCharToFile;
static void (*oldPutString)(char *) = putStringToFile;
static void (*oldPutInt)(int) = putIntToFile;
static void (*oldPutReal)(double) = putRealToFile;
static int oldMaxLength;
static int oldLength;
static char *oldBuffer;
static FILE *oldOutput;

static char *ppBuffer = 0;

void
ppPutIndent()
    {
    int i;
    for (i = 0; i < ppIndent; ++i)
        ppPutChar(' ');
    }

void
ppToString(char *buffer,int size)
    {
    //printf("setting output to string...\n");
    ppPutChar = putCharToString;
    ppPutString = putStringToString;
    ppPutInt = putIntToString;
    ppPutReal = putRealToString;
    ppBuffer = buffer;
    ppMaxLength = size;
    ppLength = 0;
    }

void
ppToFile(FILE *fp,int size)
    {
    //printf("setting output to file...\n");
    ppPutChar = putCharToFile;
    ppPutString = putStringToFile;
    ppPutInt = putIntToFile;
    ppPutReal = putRealToFile;
    ppOutput = fp;
    ppMaxLength = size;
    ppLength = 0;
    }

void
ppSaveOutput()
    {
    oldPutChar = ppPutChar;
    oldPutString = ppPutString;
    oldPutInt = ppPutInt;
    oldPutReal = ppPutReal;
    oldMaxLength = ppMaxLength;
    oldLength = ppLength;
    oldOutput = ppOutput;
    oldBuffer = ppBuffer;
    }

void
ppRestoreOutput()
    {
    ppPutChar = oldPutChar;
    ppPutString = oldPutString;
    ppPutInt = oldPutInt;
    ppPutReal = oldPutReal;
    ppMaxLength = oldMaxLength;
    ppLength = oldLength;
    ppBuffer = oldBuffer;
    ppOutput = oldOutput;
    }

static void
putCharToString(int ch)
    {
    if (ppLength > ppMaxLength - 1)
        {
        return;
        }

    if (ppLength == ppMaxLength - 4)
        {
        ppBuffer[ppLength++] = '.';
        ppBuffer[ppLength++] = '.';
        ppBuffer[ppLength++] = '.';
        ppBuffer[ppLength] = '\0';
        return;
        }

    if (ch == '\n' && ppFlat)
        {
        ppBuffer[ppLength++] = '\\';
        ppBuffer[ppLength++] = 'n';
        ppBuffer[ppLength] = '\0';
        }
    else if (ch == '\t' && ppFlat)
        {
        ppBuffer[ppLength++] = '\\';
        ppBuffer[ppLength++] = 't';
        ppBuffer[ppLength] = '\0';
        }
    else
        {
        ppBuffer[ppLength++] = ch;
        ppBuffer[ppLength] = '\0';
        }
    }

static void
putStringToString(char *s)
    {
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

    if (ppMaxLength != 0 && ppLength >= ppMaxLength - 1)
        {
        return;
        }

    if (ppMaxLength != 0 && ppLength == ppMaxLength - 3)
        {
        fprintf(ppOutput,"...");
        ppLength += 3;
        return;
        }

    if (ch == '\n' && ppFlat)
        {
        fprintf(ppOutput,"\\n");
        ppLength += 2;
        }
    else if (ch == '\t' && ppFlat)
        {
        fprintf(ppOutput,"\\t");
        ppLength += 2;
        }
    else
        {
        fprintf(ppOutput,"%c",ch);
        ++ppLength;
        }
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

