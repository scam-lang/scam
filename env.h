extern int getVariableValue(int,int);
extern int getGlobalEnvironment(void);
extern int setVariableValue(int,int,int);
extern int defineVariable(int,int,int);
extern int findLocation(int,int);

extern int makeObject(int);
extern int makeEnvironment(int,int,int,int);
extern int makeThunk(int,int);
extern int makeClosure(int,int,int,int,int);
extern int makeError(int,int,int,int,int);
extern int converThrow(int,int);
extern int makeThrow(int,int,int,int);
extern int makeBuiltIn(int,int,int,int);

extern int throw(int,int,char *,...);

#define NO_BEGIN 0
#define ADD_BEGIN 1

#define DEFINE_CELLS 2
#define EXTEND_CELLS 20

#define object_variables(x)    (cadr(x))
#define object_values(x)       (caddr(x))
#define object_label(x)        (car(object_values(x)))

#define object_variable_hook(x)(cdr(cadr(x)))
#define object_value_hook(x)   (cdr(caddr(x)))

#define OBJECT_CELLS 5
#define OBJECT_PREDEFINED 1

#define env_context(x)         (cadr(object_values(x)))
#define env_constructor(x)     (caddr(object_values(x)))
#define env_this(x)            (cadddr(object_values(x)))

#define ENV_CELLS (OBJECT_CELLS + 6)
#define ENV_PREDEFINED (OBJECT_PREDEFINED + 3)

#define thunk_context(x)       (cadr(object_values(x)))
#define thunk_code(x)          (caddr(object_values(x)))

#define THUNK_CELLS (OBJECT_CELLS + 4)
#define THUNK_PREDEFINED (OBJECT_PREDEFINED + 2)

#define closure_context(x)     (cadr(object_values(x)))
#define closure_name(x)        (caddr(object_values(x)))
#define closure_parameters(x)  (cadddr(object_values(x)))
#define closure_body(x)        (caddddr(object_values(x)))

#define CLOSURE_CELLS (OBJECT_CELLS + 8 + 1)  //one more for begin
#define CLOSURE_PREDEFINED (OBJECT_PREDEFINED + 4)

#define error_line(x)          (cadr(object_values(x)))
#define error_file(x)          (caddr(object_values(x)))
#define error_message(x)       (cadddr(object_values(x)))
#define error_trace(x)         (caddddr(object_values(x)))

#define ERROR_CELLS (OBJECT_CELLS + 8 + 2) //two more for file and line
#define ERROR_PREDEFINED (OBJECT_PREDEFINED + 4)

#define isCons(x) (type(x) == CONS)
#define isTagged(x) (isCons(x) && type(car(x)) == SYMBOL)
#define isObject(x) (isTagged(x) && ival(car(x)) == ival(objectSymbol))
#define isThunk(x) (isObject(x) && ival(object_label(x)) == ival(thunkSymbol))
#define isBuiltIn(x) (isObject(x) && ival(object_label(x)) == ival(builtInSymbol))
#define isClosure(x) (isObject(x) && ival(object_label(x)) == ival(closureSymbol))
#define isThrow(x) (isObject(x) && ival(object_label(x)) == ival(throwSymbol))
