
/*
 *  Main Author : John C. Lusth
 *  Barely Authors : Jeffrey Robinson, Gabriel Loewen
 *  Last Edit : May 4, 2014
 *
 *  The layout of this document is such:
 *      Includes
 *      external functions
 *      #defines
 */


#ifndef __ENV_H__
#define __ENV_H__

// debug
#include "pp.h"

extern int getVariableValue(int,int);
extern int getGlobalEnvironment(void);
extern int setVariableValue(int,int,int);
extern int defineVariable(int,int,int);
extern int findLocation(int,int);
extern int findLocal(int,int);

extern int makeObject(int);
extern int makeEnvironment(int,int,int,int);
extern int makeThunk(int,int);
extern int makeClosure(int,int,int,int,int);
extern int makeError(int,int,int,int);
extern int converThrow(int,int);
extern int makeThrow(int,int,int);
extern int makeBuiltIn(int,int,int,int);

extern int throw(int,char *,...);

/* 
 *  Used to handle throws  
 *  TODO : Add debugging?
 */
#define rethrow(x)              \
  do{                           \
      if(isThrow(x))            \
      {                         \
          return x;             \
      }                         \
  }while(0)

/*
 *  Used to handle throws, also pops off the stack if needed.
 *  TODO : Add debugging?
 */
#define rethrowPop(x,count)     \
  do{                           \
      if(isThrow(x))            \
      {                         \
          POPN(count);          \
          return x;             \
      }                         \
  }while(0)


/*
 *  Used to handle throws.
 *  TODO : Merge with above?
 */
#define rethrowD(x)                                                     \
    {                                                                   \
    if (isThrow(x))                                                     \
        {                                                               \
        if(Debugging)                                                   \
            {                                                           \
            P_P();                                                      \
            printf( "R : %s %d\n" ,__FILE__,__LINE__);                  \
            debugOut(stdout,0,env);                                     \
            if(isObject(env))                                           \
                {                                                       \
                int last = env;                                         \
                while (env_context(last)!=0)                            \
                    last = env_context(last);                           \
                }                                                       \
            P_V();                                                      \
            }                                                           \
        return x;                                                       \
        }                                                               \
    }


/*
 *  Used to handle throws. Also pops off stack.
 *  TODO : Merge with above?
 */
#define rethrowPopD(x,i)                                                \
    {                                                                   \
    if (isThrow(x))                                                     \
        {                                                               \
        if(Debugging)                                                   \
            {                                                           \
            printf( "R : %s %d\n" ,__FILE__,__LINE__);                  \
            debugOut(stdout,0,env);                                     \
            if(isObject(env))                                           \
                {                                                       \
                int last = env;                                         \
                while (env_context(last)!=0)                            \
                    last = env_context(last);                           \
                }                                                       \
            }                                                           \
        POPN(i);                                                        \
        return x;                                                       \
        }                                                               \
    }

/* TODO : What is this? */
#define NO_BEGIN 0
#define ADD_BEGIN 1

/* Pre-defined throws, this allows us to throw and not exit(-1) */
#define SHUTDOWN_THROW -2

/* ensureMemory sizes */
#define ADDTRACE_SIZE 1
#define DEFINE_VARIABLE_SIZE 2
#define MAKE_OBJECT_SIZE 5
#define MAKE_ENVIRONMENT_SIZE (9 + MAKE_OBJECT_SIZE)
#define MAKE_THUNK_SIZE (4 + MAKE_OBJECT_SIZE)
#define MAKE_CLOSURE_SIZE (9 + MAKE_OBJECT_SIZE)
#define MAKE_BUILTIN_SIZE MAKE_CLOSURE_SIZE
#define MAKE_THROW_SIZE (6 + MAKE_OBJECT_SIZE)

#define INTEGER_SIZE 1
#define REAL_SIZE 1
#define SYMBOL_SIZE 1

/* TODO : What is this? */
#define OBJECT_PREDEFINED 1
#define ENV_PREDEFINED (OBJECT_PREDEFINED + 4)

/* all the different kinds of object accessors */
#define object_variables(x)             (cadr(x))
#define set_object_variables(x,v)       (object_variables(x)=v)

#define object_values(x)                (caddr(x))
#define set_object_values(x,v)          (object_values(x)=v)

#define object_label(x)                 (car(object_values(x)))
#define set_object_label(x,v)           (object_label(x)=v)

#define object_variable_hook(x)         (cdr(cadr(x)))
#define set_object_variable_hook(x,v)   (object_variable_hook(x)=v)

#define object_value_hook(x)            (cdr(caddr(x)))
#define set_object_value_hook(x,v)      (object_value_hook(x)=v)

#define env_context(x)          (cadr(object_values(x)))
#define set_env_context(x,v)    (env_context(x)=v)

#define env_level(x)            (caddr(object_values(x)))
#define set_env_level(x,v)      (env_level(x)=v)

#define env_constructor(x)      (cadddr(object_values(x)))
/* in the macros below */
/* the # of d's in cddddr should be ENV_PREDEFINED - 1 */
#define env_variable_hook(x)    (cddddr(object_variables(x)))
#define env_value_hook(x)       (cddddr(object_values(x)))

#define thunk_context(x)        (cadr(object_values(x)))
#define thunk_code(x)           (caddr(object_values(x)))

#define closure_context(x)      (cadr(object_values(x)))
#define closure_name(x)         (caddr(object_values(x)))
#define closure_parameters(x)   (cadddr(object_values(x)))
#define closure_body(x)         (caddddr(object_values(x)))

#define error_code(x)           (cadr(object_values(x)))
#define set_error_code(x,v)     (error_code(x)=v)

#define error_value(x)          (caddr(object_values(x)))
#define set_error_value(x,v)    (error_value(x)=v)

#define error_trace(x)          (cadddr(object_values(x)))
#define set_error_trace(x,v)    (error_trace(x)=v)

/* macros for quickly finding types */

#define isCons(x) (type(x) == CONS)
#define isTagged(x) (isCons(x) && type(car(x)) == SYMBOL)
#define isObject(x) (isTagged(x) && ival(car(x)) == ival(ObjectSymbol))
#define isThunk(x) (isObject(x) && ival(object_label(x)) == ival(ThunkSymbol))
#define isEnv(x) (isObject(x) && ival(object_label(x)) == ival(EnvSymbol))
#define isClosure(x) (isObject(x) && \
    ((ival(object_label(x)) == ival(ClosureSymbol)) \
    || (ival(object_label(x)) == ival(BuiltInSymbol))))
#define isThrow(x) ((isObject(x) && ival(object_label(x)) == ival(ThrowSymbol)) || (x)==SHUTDOWN_THROW)
#define isError(x) (isObject(x) && ival(object_label(x)) == ival(ErrorSymbol))
#define isReturn(x) (isThrow(x) && ival(error_code(x)) == ival(ReturnSymbol))
#define isBuiltIn(x) (isClosure(x) && object_label(x)== BuiltInSymbol)

#endif
