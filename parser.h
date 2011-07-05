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

extern int parse(PARSER *);
extern PARSER *newParser(char *);
extern void freeParser(PARSER *);
