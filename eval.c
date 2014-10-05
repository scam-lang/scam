
/*
 *  Main Author : John C. Lusth
 *  Barely Authors : Jeffrey Robinson, Gabriel Loewen
 *  Last Modified : May 4, 2014
 *
 *  TODO : Description
 *
 */

#include  <stdio.h>

#include "scam.h"
#include "cell.h"
#include "types.h"
#include "env.h"
#include "eval.h"
#include "prim.h"

int processPassThroughArguments(int,int,int,int,int);
int processNoEvaluationArguments(int,int,int,int,int,int);
int processArguments(int,int,int,int,int,int);
int unevaluatedList(int);
void makeTraceEntry(int,int);
int evaluatedList(int,int);

#ifdef DEBUG
    int depth[MAX_THREADS] = {0};

    #define INC(STR,EXPR) \
    do { \
        P_P();                              \
        int S_I = THREAD_ID;                \
        depth[S_I]++;                       \
        int i = depth[S_I];                 \
        printf("%d",S_I);                   \
        while(i)                            \
        {                                   \
            printf( " " );                  \
            --i;                            \
        }                                   \
        printf("%d\t",depth[S_I]);          \
        debug(STR,EXPR);                    \
        P_V();                              \
    }while(0)

    #define DEC(STR)                        \
        do{                                 \
        P_P();                              \
        int S_I = THREAD_ID;                \
        int i = depth[S_I];                 \
        printf("%d",S_I);                   \
        while( i )                          \
        {                                   \
            printf( " " );                  \
            --i;                            \
        }                                   \
        printf( "%d\t" ,depth[S_I] );       \
        printf("Leaving %s\n",STR);         \
        depth[S_I]--;                       \
        P_V();                              \
        }while(0)
#else
    #define INC(A,B)
    #define DEC(A)
#endif


/*
 * eval - the main evaluator function
 *      - it is implemented as a loop so tail recursion can be optimized
 *      - if an evaluation returns a thunk, it is unpacked and the loop continues
 */

int
eval(int expr, int env)
    {
    int result;
    if (ShuttingDown)
        {
        P_P();
        printf("EVAL : ShuttingDown\n");
        P_V();
        return SHUTDOWN_THROW;
        }
    INC("eval",expr);
    /* level is used to decide when returns should stop */
    int level = ival(env_level(env));

    int returns = 0; /* tracks how many levels have been broken through */

    while(1)
        {
        int tag;
        char *etype;

        if (expr == 0)
            {
            result = 0;
            break;
            }

        etype = type(expr);
        /* check for expressions that evaluate to themselves */
        /* a symbol can be nil, #t, #f, or a variable */
        if (etype == SYMBOL)
            {
            int index = ival(expr);
            if (index == trueIndex)
                {
                result = TrueSymbol;
                }
            else if (index == falseIndex)
                {
                result = FalseSymbol;
                }
            else if (index == nilIndex)
                {
                result = 0;
                }
            else
                {
                result = getVariableValue(expr,env);
                rethrow(result);
                }
            break; /* out of the eval loop */
            }

        /* String, Integer, Real */
        if (etype != CONS)
            {
            result = expr;
            break;
            }

       /* must be a list at this point */

        tag = car(expr); /* the tag tells us what kind of list/object */

        /* objects evaluate to themselves */

        if (SameSymbol(tag,ObjectSymbol))
            {
            result = expr;
            break; /* out of the eval loop */
            }

        /* must be a function call at this point */
        PUSH(expr);
        result = evalCall(expr,env,NORMAL);
        expr = POP();
        /* returns are handled as exceptions */

        /* Explicit return called */
        if (isReturn(result))
            {
            /* Get arguments to return */
            int s = error_value(result);

            /* Two cases for return.  Either from tail recursion or */
            /* returned from middle of function */

            if (level < (ival(env_level(thunk_context(s))) - returns))
                {
                /* Unwrapping nested returns*/
                result = s; // result is now a thunk
                returns++; // Used to keep track of how far we need to unwrap
                }
            else
                {
                /* Handle return for this environment level */
                ival(env_level(thunk_context(s)))
                    = ival(env_level(thunk_context(s))) - returns; 
                }
            }
        else if (isThrow(result))
            {
            PUSH(result);
            makeTraceEntry(expr,result); /* add current expr to the backtrace */
            result = POP();
            break; /* out of the eval loop */
            }

        /* if the result is not a thunk, there is no tail recursion to unpack */
        /* so we are done at this eval level */

        if (!isThunk(result))
            {
            break; /* out of the eval loop */
            }

        /* result is a thunk, so unpack and re-evaluate */

        expr = thunk_code(result);
        env = thunk_context(result);
                
        /* keep the level at lowest setting to properly handle returns */

        /* Jumping into a new environment, the level might change when we  */
        /* call builtin function eval. */

        if (level > ival(env_level(env)))
            {
            level = ival(env_level(env));
            }
        }

    DEC("eval");
    return result;
    }
        
/* evalCall - evaluate a built-in or user defined function
 *          - the call will not actually be made if user defined
 */

int
evalCall(int call,int env, int mode)
    {
    INC( "evalCall" , call );
    int closure,eargs; int callingLevel = ival(env_level(env));

    /* NORMAL mode unless apply is being called */

    if (Caching && isClosure(car(call)))
        {
        closure = car(call);
        }
    else if (mode == NORMAL)
        {
        PUSH(call);
        PUSH(env);
        closure = eval(car(call),env);
        env = POP();
        call = POP();
        rethrow(closure); // All returns are thrown
        }
    else
        {
        /* everything has already been evaluated (apply) */
        closure = car(call);
        }

    /* args are the cdr of call */

    /* check if user-defined or built-in */

    if (isClosure(closure)) /* true if user-defined or built-in */
        {
        int result;

        /* evaluate the arguments */
        char *b_t = type(closure);
        int   b_i = ival(closure);
        int   b_c = cdr(closure);

        PUSH(closure);
        PUSH(call);
        switch(mode)
             {
             case PASS_THROUGH:
                eargs = processPassThroughArguments(
                    closure_name(closure),
                    closure_parameters(closure),
                    cdr(call),
                    file(call),
                    line(call)
                    );
                break;
            case NO_EVALUATION:
                eargs = processNoEvaluationArguments(
                    closure_name(closure),
                    closure_parameters(closure),
                    cdr(call),
                    env,
                    file(call),
                    line(call)
                    );
                break;
            default:
                eargs = processArguments(
                    closure_name(closure),
                    closure_parameters(closure),
                    cdr(call),
                    env,
                    file(call),
                    line(call)
                    );
                break;
            }
        /* Everyone does this */
        call = POP();
        closure = POP();

        char *a_t = type(closure);
        int   a_i = ival(closure);
        int   a_c = cdr(closure);

        if ( (a_t != b_t) || (b_i != a_i))
            {
            P_P();
            printf("closure changed after processing arguments!\n");
            printf("Type: %s changed to %s\n",b_t,a_t);
            printf("ival: %d changed to %d\n",b_i,a_i);
            printf("cdr : %d changed to %d\n",b_c,a_c);
            P_V();
            Fatal("closure changed after processing arguments!");
            }
        rethrow(eargs);

        if (SameSymbol(object_label(closure),BuiltInSymbol))
            {
            /* it's a builtin, so extract the function pointer from the
             * closure body and call it
             */
            if (Caching && !isClosure(car(call)))
                {
                setcar(call,closure);
                }
            PRIM prim = BuiltIns[ival(closure_body(closure))];
            result = prim(eargs);
            }
        else /* it's a user-defined closure */
            {
            int params,body,xenv;
            //TODO: speed up this test somehow?
            if (Caching && !isClosure(car(call)))
                {
                setcar(call,closure);
                }
            P();
            ENSURE_MEMORY(MAKE_ENVIRONMENT_SIZE
                    + INTEGER_SIZE \
                    + MAKE_THUNK_SIZE,
                    &closure,
                    &eargs,
                    (int*) 0);
            params = closure_parameters(closure);
            body = closure_body(closure);
            xenv = closure_context(closure);
            xenv = makeEnvironment(xenv,closure,params,eargs);
            set_env_level(xenv,newIntegerUnsafe(callingLevel + 1));

            /* the extended level is one plus the calling level (for returns) */

            result = makeThunk(body,xenv);
            V();
            }
        DEC("evalCall");
        return result;
        }
    else if (isObject(closure)) /* to handle things like x.y.z (x'y'z) */
        {
        PUSH(closure);
        eargs = evaluatedList(cdr(call),env);
        closure = POP();

        rethrow(eargs); //need DEC after all rethrows (move into rethrow)

        while(eargs != 0)
            {
            if (!isObject(closure))
                {
                DEC("evalCall");
                return throw(NonObjectSymbol,
                        "file %s,line %d: "
                        "attempted to access %s as an object",
                        SymbolTable[file(closure)],
                        line(closure),
                        type(closure)
                    );
                }
            closure = getVariableValue(car(eargs),closure);
            rethrow(closure);
            eargs = cdr(eargs);
            }
        DEC("evalCall");
        return closure;
        }
    else
        {
        DEC("evalCall");
        return throw(NonFunctionSymbol,
            "file %s,line %d: "
            "attempted to call %s as a function",
            SymbolTable[file(closure)],line(closure),
            type(closure));
        }
    }

/* evalBlock - evaluate the list of expressions as a block of statements
 *
 */

int
evalBlock(int items,int env,int mode)
    {
    INC( "evalBlock" , items );
    int result = 0;

    PUSH(items);
    #define EB_ITEMS 1
    PUSH(env);
    #define EB_ENV 0

    /* evaluate all but the last one */
    while(cdr(PEEK(EB_ITEMS)) != 0)
        {
        result = eval(car(PEEK(EB_ITEMS)),PEEK(EB_ENV));
        if (isThrow(result))
            {
            if (!isReturn(result))
                {
                PUSH(result);
                makeTraceEntry(car(PEEK(EB_ITEMS)),result);
                result = POP();
                }
            DEC("evalBlock");
            POPN(2); /* pop of env and items */
            return result;
            }
        /* go to the next item */
        REPLACE(EB_ITEMS,cdr(PEEK(EB_ITEMS)));
        }

    /* evaluate the last one? */
    if (mode == ALLBUTLAST)
        {
        P();
        ENSURE_MEMORY(MAKE_THUNK_SIZE + 1,(int *) 0);
        result = makeThunk(car(PEEK(EB_ITEMS)),PEEK(EB_ENV));
        V();
        }
    else /* go ahead an evaluate the last expression */
        {
        result = eval(car(PEEK(EB_ITEMS)),PEEK(EB_ENV));
        if (isThrow(result) && !isReturn(result))
            {
            PUSH(result);
            makeTraceEntry(car(PEEK(EB_ITEMS)),result); /* peek items */
            result = POP();
            }
        }

    POPN(2); /* env and items */
    DEC("evalBlock");
    return result;
    }


/*************** PRIVATE FUNCTIONS **************************************/

/*
 * makeTraceEntry - add an expression to the backtrace of an exception
 *
 */

void
makeTraceEntry(int item,int error)
    {
    INC("makeTraceEntry",item);

    int et;

    

    /* caller is responsible for ensuring enough memory */

    if (type(item) == CONS && SameSymbol(car(item),BeginSymbol)) 
        {
        DEC("makeTraceEntry");
        return;
        }

    P();
    ENSURE_MEMORY(ADDTRACE_SIZE,&item,&error,(int *) 0);
    et = error_trace(error);

    if (error_trace(error) == 0)
        {
        set_error_trace(error,cons(item,0));
        }
    else if (file(item) != file(car(et)) || line(item) != line(car(et)))
        {
        set_error_trace(error,cons(item,et));
        }
    V();
    DEC("makeTraceEntry");
    }

int
processPassThroughArguments(int name,int params,int args,int fi,int li)
    {
    INC("processPassThroughArguments",params);

    int acount = length(args);
    int pcount = length(params);

    if (acount < pcount)
        {
        args = throw(ExceptionSymbol,
            "file %s,line %d: "
            "too few arguments to function %s",
            SymbolTable[fi],li,
            SymbolTable[ival(name)]);
        }
    else if (pcount < acount)
        {
        args = throw(ExceptionSymbol,
            "file %s,line %d: "
            "too many arguments to function %s",
            SymbolTable[fi],li,
            SymbolTable[ival(name)]);
            }
    DEC("processPassThroughArguments");
    return args;
    }

int
processNoEvaluationArguments(int name,int params,int args,int env,int fi,int li)
    {
    INC("processNoEvaluationArguments",params);
    int result;
    int pos;
    int callingEnvBound = 0;

    P();
    ENSURE_MEMORY(1,&name,&params,&args,&env,(int *) 0);
    result = cons(0,0); /* dummy head pointer, result is actually in cdr(result) */
    V();
    pos = result;

    while(args || params)
        {
        if (params == 0)
            {
            DEC("processNoEvaluationArguments");
            return throw(ExceptionSymbol,
                "file %s,line %d: "
                "too many arguments to function %s",
                SymbolTable[fi],li,
                SymbolTable[ival(name)]);
            }
        if (SameSymbol(car(params),AtSymbol)) /* evaluated variadic */
            {
            int rest = unevaluatedList(args);
            P();
            ENSURE_MEMORY(1,&rest,&name,&params,&args,&env,(int *) 0);
            int o = consfl(rest,0,fi,li);
            V();
            setcdr(pos,o);
            break; //out of the while loop
            }
        else if (SameSymbol(car(params),DollarSymbol)) /* delayed variadic */
            {
            int rest = unevaluatedList(args);
            P();
            ENSURE_MEMORY(1,&rest,&name,&params,&args,&env,(int *) 0);
            int o = consfl(rest,0,fi,li);
            V();
            setcdr(pos,o);
            break; //out of the while loop
            }
        else if (SameSymbol(car(params),SharpSymbol)) /* capture calling env */
            {
            params = cdr(params);
            if (callingEnvBound == 0)
                {
                P();
                ENSURE_MEMORY(1,&name,&params,&args,&env,(int *) 0);
                int o = consfl(env,0,fi,li);
                V();
                callingEnvBound = 1;
                setcdr(pos,o);
                pos = cdr(pos);
                }
            }
        else if (args == 0) /* mismatch between args and params */
            {
            DEC("processNoEvaluationArguments");
            return throw(ExceptionSymbol,
                    "file %s,line %d: "
                    "too few arguments to function %s",
                    SymbolTable[fi],li,
                    SymbolTable[ival(name)]);
            }
        else /* a normal arg and delayed arg are treated the same */
            {
            P();
            ENSURE_MEMORY(1,&name,&params,&args,&env,(int *) 0);
            int o = consfl(car(args),0,fi,li);
            V();
            setcdr(pos,o); 
            params = cdr(params);
            args = cdr(args);
            pos = cdr(pos);
            }
        }
    DEC("processNoEvaluationArguments");
    return cdr(result); /* skip over dummy head node */
    }

/* processArguments - delay, variadic, capture the calling env,
 *                    processArguments does it all!
 *
 *  Not heap safe
 */

int
processArguments(int name,int params,int args,int env,int fi,int li)
    {
    INC("processArguments",name);

    /* Plan : Build up from the beginning adding to some position pointer
     *        then return a saved pointer to the head.
     */
    int first,rest,result;
    int callingEnvBound = 0;
    if (params == 0 && args == 0)
        {
        DEC("processArguments");
        return 0;
        }

    PUSH(name);     /* peek location 5 */
    #define PA_NAME 5
    PUSH(params);   /* peek location 4 */
    #define PA_PARAMS 4
    PUSH(args);     /* peek location 3 */
    #define PA_ARGS 3
    PUSH(env);      /* peek location 2 */
    #define PA_ENV 2

    P();
    ENSURE_MEMORY(1,(int *) 0);
    result = cons(0,0); /* dummy head pointer, actual result is cdr(result) */
    V();

    int hook = result; /* where to hook on the next argument */

    PUSH(result);   /* peek location 1 */
    #define PA_RESULT 1
    PUSH(hook);     /* peek location 0 */
    #define PA_HOOK 0

    while(PEEK(PA_PARAMS) || PEEK(PA_ARGS))
        {
        if (PEEK(PA_PARAMS) == 0)
            {
            name = PEEK(PA_NAME);
            POPN(6);
            DEC("processArguments");
            return throw(ExceptionSymbol,
                    "file %s,line %d: "
                    "too many arguments to function %s",
                    SymbolTable[fi],li,
                    SymbolTable[ival(name)]);
            }

        if (SameSymbol(car(PEEK(PA_PARAMS)),AtSymbol))
            {
            /* evaluated variadic */
            rest = evaluatedList(PEEK(PA_ARGS),PEEK(PA_ENV));
            rethrowPop(rest,6); /* everything */
            P();
            ENSURE_MEMORY(1,&rest,(int *) 0);
            int o = consfl(rest,0,fi,li);
            V();
            setcdr(PEEK(PA_HOOK),o);
            REPLACE(PA_HOOK,cdr(PEEK(PA_HOOK)));
            /* STACKCHECK */
            /* UPDATE(PA_RESULT); */
            break;
            }
        else if (SameSymbol(car(PEEK(PA_PARAMS)),DollarSymbol))
            {
            /* delayed variadic */
            /* length(PEEK(PA_ARGS)) is the memory unevaluatedList needs */
            rest = unevaluatedList(PEEK(PA_ARGS));
            P();
            ENSURE_MEMORY(1,&rest,(int *) 0);
            int o = consfl(rest,0,fi,li);
            V();
            setcdr(PEEK(PA_HOOK),o);
            REPLACE(PA_HOOK,cdr(PEEK(PA_HOOK)));
            /* STACKCHECK */
            /* UPDATE(PA_RESULT); */
            break;
            }
        else if (SameSymbol(car(PEEK(PA_PARAMS)),SharpSymbol))
            {
            /* capture calling env */
            if (callingEnvBound == 0)
                {
                P();
                ENSURE_MEMORY(1,(int *) 0);
                int o = consfl(PEEK(PA_ENV),0,fi,li);
                V();
                setcdr(PEEK(PA_HOOK),o);
                REPLACE(PA_HOOK,cdr(PEEK(PA_HOOK)));
                /* STACKCHECK */
                /* UPDATE(PA_RESULT); */
                callingEnvBound = 1;
                }
            REPLACE(PA_PARAMS,cdr(PEEK(PA_PARAMS)));
            }
        else if (PEEK(PA_ARGS) == 0)
            {
            /* mismatch between args and params */
            name = PEEK(PA_NAME);
            POPN(6);  /* everything */
            DEC("ProcessArguments");
            return throw(ExceptionSymbol,
                    "file %s,line %d: "
                    "too few arguments to function %s",
                    SymbolTable[fi],li,
                    SymbolTable[ival(name)]);
            }
        else if (*SymbolTable[ival(car(PEEK(PA_PARAMS)))] == '$')
            {
            P();
            /* delay one arg */
            ENSURE_MEMORY(1,(int *) 0);
            int o = consfl(car(PEEK(PA_ARGS)),0,fi,li);
            V();
            setcdr(PEEK(PA_HOOK),o);
            REPLACE(PA_HOOK,cdr(PEEK(PA_HOOK)));
            REPLACE(PA_PARAMS,cdr(PEEK(PA_PARAMS)));
            REPLACE(PA_ARGS,cdr(PEEK(PA_ARGS)));
            /* STACKCHECK */
            /* UPDATE(PA_RESULT); */
            }
        else /* else normal arg/param match - evaluate (except if apply) */
            {
            first = eval(car(PEEK(PA_ARGS)),PEEK(PA_ENV));
            rethrowPop(first,6);    /* everything */

            /* transfer trace info */
            setfile(first,file(car(PEEK(PA_ARGS))));
            setline(first,line(car(PEEK(PA_ARGS))));
            
            P();
            ENSURE_MEMORY(1,&first,(int *) 0);
            int o = consfl(first,0,fi,li);
            V();

            setcdr(PEEK(PA_HOOK),o);
            REPLACE(PA_HOOK,cdr(PEEK(PA_HOOK)));
            REPLACE(PA_PARAMS,cdr(PEEK(PA_PARAMS)));
            REPLACE(PA_ARGS,cdr(PEEK(PA_ARGS)));
            /* STACKCHECK */
            /* UPDATE(PA_RESULT); */
            }
        }

    result = cdr(PEEK(PA_RESULT));
    POPN(6); /* everything */
    DEC("processArguments");
    return result;
    }

/* evaluatedList - maps eval over a list of expressions
 */

int
evaluatedList(int items,int env)
    {
    INC( "evaluatedList" , items );
    int fi;
    int li;
    int result = 0;
    int hook = 0;


    if (items == 0) 
        {
        DEC("evaluatedList");
        return 0;
        }

    /* evaluate the car separately so we have a place to add subsequent
     * expressions - this allows us to use a loop rather than recursion
     */

    fi = file(car(items));
    li = line(car(items));

    PUSH(items); /* peek location 1 */
    #define EL_ITEMS 1
    PUSH(env); /* peek location 0 */
    #define EL_ENV 0

    result = eval(car(items),env); /* no need to peek here */
    rethrowPop(result,2);
    
    P();
    ENSURE_MEMORY(1,&result,(int *) 0);
    result = consfl(result, 0, fi,li );
    V();
    
    /* spot is our hook - it's the last cell in the chain */
    
    hook = result;
    REPLACE(EL_ITEMS,cdr(PEEK(EL_ITEMS)));

    /* undefine these because of subsequent pushes */
    #undef EL_ITEMS
    #undef EL_ENV 

    #define EL_ITEMS 3
    #define EL_ENV 2
    PUSH(result); /* save the start of the list for the final return */
    #define EL_RESULT 1
    PUSH(hook);  /* peek location 0 */
    #define EL_HOOK 0

    while(PEEK(EL_ITEMS) != 0)
        {
        /*
         *  Heap items that can be modified : 
         *      env
         *      items
         *      result
         *      hook
         */

        int value;

        value = eval(car(PEEK(EL_ITEMS)),PEEK(EL_ENV));

        /* Debug macros DEC/INC are broken here */
        rethrowPop(value,4);
       
        P();
        ENSURE_MEMORY(1,&value,(int *) 0);
        value = cons(value,0);
        V();

        setcdr(PEEK(EL_HOOK),value);
        REPLACE(EL_HOOK,value);
        REPLACE(EL_ITEMS,cdr(PEEK(EL_ITEMS)));
        /* STACKCHECK */
        /* UPDATE(EL_RESULT); */
        }
    result = PEEK(EL_RESULT);
    POPN(4); /* everything */

    DEC("evaluatedList");
    return result;
    }

/* unevaluatedList - make a copy of a list of expressions
 *                 - it is up to the caller to ensure (length args) memory
 *                   is available
 */

int
unevaluatedList(int args)
    {
    INC("unevaluatedList",args);
    int result,pos;

    /* it is up to the caller to ensure (length args) memory is available */

    if (args == 0)
        {
        DEC("unevaluatedList");
        return 0;
        }

    P();
    ENSURE_MEMORY(length(args),&args,(int *)0);

    result = consfl(car(args), 0, file(car(args)), line(car(args)));

    pos = result;

    args = cdr(args);    
    while(args != 0)
        {
        int TMP = car(args);
        TMP = consfl( TMP , 0 , file(TMP), line(TMP) );
        setcdr(pos,TMP);
        pos = TMP;
        args = cdr(args);
        }
    V();
    DEC("unevaluatedList");
    return result;
    }
