// Java style inheritance in three functions (really two, super is not needed)
// linear time

include("reflection.s");

function .(obj,$field) { get($field,obj); }

function cddddr(x) { cdr(cdr(cdr(cdr(x)))); }

function class(x) { x . label; }

function closure?(x)
    {
    pair?(x) && car(x) == :object && class(x) == :closure;
    }

function object?(x)
    {
    pair?(x) && car(x) == :object;
    }

function member?(x,items)
    {
    if (null?(items))
        {
        #f;
        }
    else if (x == car(items))
        {
        #t;
        }
    else
        {
        member?(x,cdr(items));
        }
    }

function local?(id,env)
    {
    member?(id,car(cdr(env)));
    }

function locals(env)
    {
    cddddr(car(cdr(cdr(env))));
    }

function resetClosures(static,obj)
    {
    function update(current,rest)
        {
        if (closure?(current))
            {
            //println("setting context of ",current," to ",static);
            current . context = static;
            }
        if (rest != nil) { update(car(rest),cdr(rest)); }
        }
    var values = locals(obj);
    //println("locals are ",values);
    if (values != nil) { update(car(values),cdr(values)); }
    obj;
    }

function inherit(child,parents,reification,static)
    {
    if (null?(parents))
        {
        child . context = static;
        }
    else
        {
        child . context =
                inherit(resetClosures(reification,car(parents)),
                       cdr(parents),reification,static);
        }
    child;
    }

function new(child)
    {
    function chain(x) { if (null?(x),nil,cons(x,chain(x . parent))); }
    inherit(child,chain(child . parent),child,child . context);
    }

function mixin(object,@)
    {
    inherit(object,@,object,object . context);
    }

function super(child)
    {
    child . context;
    }

function extend(#,parent)
    {
    var top = if(local?(:inherit-top,parent),parent . inherit-top,parent);
    
    //println("in extend...");
    addSymbol(:inherit-top,top,#);
    
    top  . context = # . context;
    # . context = parent;
    #;
    }
