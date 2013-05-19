// mini parser for alpha-substitution

function rewrite-define(old,new,names)
    {
    names head= cons(list(old,new),head(names));
    new;
    }

// extend the name env, binding original names to themselves

function rewrite-extend(params,values,names)
    {
    cons(make-assoc(params,values),names);
    }

// rewrite a variable definition

function rewrite-variable-definition(id,expr,names,free)
    {
    var newid = if (gensym?(id),id,rewrite-define(id,gensym(id),names));
    // add the identifier to names
    cons(:define,
        cons(newid, 
            rewrite(cddr(expr),names,free)));
    }

// rewrite a function definition

function rewrite-function-definition(sig,expr,names,free)
    {
    var id  = car(sig);
    var newid = if (gensym?(id),id,rewrite-define(id,gensym(id),names));
    var next  = rewrite-extend(cdr(sig),cdr(sig),names);
    // add function name to name environment
    cons(:define,
        cons(cons(newid,rewrite(cdr(sig),next,free)),
            rewrite(cddr(expr),next,free)));
    }

// dispatch function for generic definitions

function rewrite-definition(expr,names,free)
    {
    if (symbol?(cadr(expr)))
        {
        rewrite-variable-definition(cadr(expr),expr,names,free);
        }
    else
        {
        rewrite-function-definition(cadr(expr),expr,names,free);
        }
    }

function rewrite-scope(expr,names,free)
    {
    cons(:scope,rewrite(cdr(expr),rewrite-extend(null,null,names),free));
    }

// look up bindings in the name env
//   free variables

function rewrite-get(id,names,free)
    {
    //println("rewrite-get: id: ",id," name: ",names);
    var target;
    if (gensym?(id))
        {
        id;
        }
    else if (null?(names) && null?(free))
        {
        id;
        }
    else if (null?(names))
        {
        get(id,free);           //free variable
        }
    else
        {
        target = assoc(id,car(names));
        if (not(null?(target)))
            {
            target;   //bound variable
            }
        else
            {
            rewrite-get(id,cdr(names),free);
            }
        }
    }

function rewrite(expr,names,free)
    {
    //println("rewrite: expr: ",expr," names: ",names);
    if (null?(expr))
        {
        null;
        }
    else if (symbol?(expr))
        {
        rewrite-get(expr,names,free);
        }
    else if (define?(expr))
        {
        rewrite-definition(expr,names,free);
        }
    else if (scope?(expr))
        {
        rewrite-scope(expr,names,free);
        }
    else if (object?(expr) || atom?(expr))
        {
        expr;
        }
    else
        {
        cons(rewrite(car(expr),names,free),rewrite(cdr(expr),names,free));
        }
    }

// compilation and macro functions ;;;;;;;;;;;;;;;;;;;;;;;

function compile(f,env)
    {
    set!(:code,
        rewrite(
            get(:code,f),
            rewrite-extend(get(:parameters,f),get(:parameters,f),null),
            env
            ),
        f);
    f;
    }

function macro(#,f,$)
    {
    // # is bound to the calling environment
    // $ is bound to a list of unevaluated arguments
    var names = list(make-assoc(get(:parameters,f),$));
    var body = rewrite(get(:code,f),names,get(:context,f));
    eval(body,#);
    }
