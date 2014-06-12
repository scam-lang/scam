extern int ppIndent;
extern int ppFlat;
extern int ppLength;
extern int ppMaxLength;

extern FILE *ppOutput;

extern void ppToString(char *,int);
extern void ppToFile(FILE *,int);

extern void (*ppPutChar)(int);
extern void (*ppPutString)(char *);
extern void (*ppPutInt)(int);
extern void (*ppPutReal)(double);

extern void ppPutIndent(void);
extern void ppSaveOutput(void);
extern void ppRestoreOutput(void);

#define ppFLAT 1
