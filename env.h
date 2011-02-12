extern int makeEnvironment(int,int,int);
extern int lookupVariableValue(int,int);
extern int getGlobalEnvironment(void);
extern int setVariableValue(int,int,int);
extern int defineVariable(int,int,int);
extern int findLocation(int,int);

extern int makeThunk(int,int);
extern int makeClosure(int,int,int,int,int);
extern int makeObject(int,int,int,int);
extern int makeError(int,int,int,int,int,int);
extern int makeError_from_error(int,int);
extern int makeThrow(int,int,int);
extern int makeBuiltIn(int,int,int,int);

#define NO_BEGIN 0
#define ADD_BEGIN 1

#define DEFINE_CELLS 2
#define EXTEND_CELLS 20

#define closure_variables(x)   (cadr(x))
#define closure_values(x)      (caddr(x))
#define closure_context(x)     (car(closure_values(x)))
#define closure_name(x)        (cadr(closure_values(x)))
#define closure_parameters(x)  (caddr(closure_values(x)))
#define closure_body(x)        (cadddr(closure_values(x)))

#define CLOSURE_CELLS 12

#define thunk_variables(x)     (cadr(x))
#define thunk_values(x)        (caddr(x))
#define thunk_context(x)       (car(thunk_values(x)))
#define thunk_code(x)          (cadr(thunk_values(x)))

#define THUNK_CELLS 7

#define object_variables(x)    (cadr(x))
#define object_values(x)       (caddr(x))
#define object_context(x)      (car(object_values(x)))
#define object_constructor(x)  (cadr(object_values(x)))
#define object_this(x)         (caddr(object_values(x)))

#define OBJECT_CELLS 9

#define error_variables(x)     (cadr(x))
#define error_values(x)        (caddr(x))
#define error_context(x)       (car(error_values(x)))
#define error_code(x)          (cadr(error_values(x)))
#define error_type(x)          (caddr(error_values(x)))
#define error_value(x)         (cadddr(error_values(x)))
#define error_trace(x)         (caddddr(error_values(x)))

#define ERROR_CELLS 13

#define isClosure(x)  ((type(x) == CONS && sameSymbol(car(x),closureSymbol)))
#define isBuiltIn(x)  ((type(x) == CONS && sameSymbol(car(x),builtInSymbol)))
#define isObject(x)   ((type(x) == CONS && sameSymbol(car(x),objectSymbol)))
#define isThunk(x)    ((type(x) == CONS && sameSymbol(car(x),thunkSymbol)))
#define isError(x)    ((type(x) == CONS && sameSymbol(car(x),errorSymbol)))

#define ENV_PREDEFINED 3

