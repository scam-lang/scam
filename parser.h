typedef struct parsertag
    {
    char *fileName;
    int pending;
    int file;
    int line;
    int pushBack;
    int pushedBack;
    FILE *input;
    FILE *output;
    } PARSER;

#define SCAM 1
#define SWAY 2

extern int scamParse(PARSER *);
extern int swayParse(PARSER *);
extern PARSER *newParser(char *);
extern void freeParser(PARSER *);
