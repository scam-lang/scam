extern int ppIndent;
extern int ppFlatLimit;
extern int ppLength;
extern int ppFlags;

extern FILE *ppOutput;

extern void ppToString(char *,int);
extern void ppToFile(FILE *);

extern void (*ppPutChar)(int);
extern void (*ppPutString)(char *);
extern void (*ppPutInt)(int);
extern void (*ppPutReal)(double);

#define ppFLAT 1
