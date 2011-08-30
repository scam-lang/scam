extern void pp(FILE *,int);
extern void ppLevel(FILE *,int,int);
extern void ppObject(FILE *,int,int);
extern void ppList(FILE *,char *,int,char *,int);
extern void ppArray(FILE *,char *,int,char *,int);
extern void ppCons(FILE *,int,int);
extern void ppTable(FILE *,int,int);
extern void debug(char *,int);

extern int ppQuoting;
