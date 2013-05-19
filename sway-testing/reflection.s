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

function local?(id,env)
    {
    member?(id,car(cdr(env)));
    }

function defined?(id,env)
    {
    var result = catch(get(id,env));
    return type(result) != :ERROR;
    }

function locals(env)
    {
    cddddr(car(cdr(cdr(env))));
    }
