typedef struct parsertag
    {
    char *fileName;
    int pending;
    int fileIndex;
    int lineNumber;
    FILE *input;
    FILE *output;
    } PARSER;

extern int parse(PARSER *);
extern PARSER *newParser(char *);
