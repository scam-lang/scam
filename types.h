/* xcheme punctuation */

extern char *CLOSE_PARENTHESIS;// = "CLOSE_PARENTHESIS";
extern char *OPEN_PARENTHESIS;// = "OPEN_PARENTHESIS";
extern char *QUOTE;//= "QUOTE";
extern char *BACKQUOTE;//= "QUOTE";
extern char *COMMA;// = "COMMA";
extern char *END_OF_INPUT;// = "END_OF_INPUT";

/* xcheme keywords */

extern char *BAD_NUMBER;// = "BAD_NUMBER";

/* xcheme internal types */

extern char *INTEGER;// = "INTEGER";
extern char *REAL;// = "REAL";
extern char *STRING;// = "STRING";
extern char *SYMBOL;// = "SYMBOL";
extern char *CONS;// = "THUNK";
extern char *PAST;// = "PAST";
extern char *FUTURE;// = "FUTURE";

/* error codes */

#define BAD_CHARACTER_CODE      1
#define UNDEFINED_VARIABLE      2
#define UNTERMINATED_STRING     3
#define UNTERMINATED_COMMENT    4
#define STRING_TOO_LARGE        5
#define SYMBOL_TOO_LARGE        6
#define NUMBER_TOO_LARGE        7
#define OUT_OF_MEMORY           8
#define STACK_OVERFLOW          9
#define TOO_FEW_ARGUMENTS      10
#define TOO_MANY_ARGUMENTS     11
#define INTERNAL_ERROR         12
#define USAGE_ERROR            13
