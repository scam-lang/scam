function head(x) { car(x); }
function tail(x) { cdr(x); }
function join(x,y) { cons(x,y); }

function for(init, #, $test, $increment, $body)
    {
    while(eval($test,#))
        {
        eval($body,#);
        eval($increment,#);
        }
    }

function for-each(#,$indexVar,items,$body)
    {
    var result = false;
    while (items != null)
        {
        set!($indexVar,head(items),#);
        result = eval($body,#);
        items = tail(items);
        }
    return result;
    }

function +=($v,value,#) { set!($v,eval($v,#) + value,#); }
function -=($v,value,#) { set!($v,eval($v,#) - value,#); }
function *=($v,value,#) { set!($v,eval($v,#) * value,#); }
function /=($v,value,#) { set!($v,eval($v,#) / value,#); }

var old-type = type;
function type(x)
    {
    if (object?(x),return x . label);
    return old-type(x);
    }

function .(obj,$field) { get($field,obj); }

function assoc(a,items)
    {
    if (null?(items))
        {
        null;
        }
    else if (a == caar(items))
        {
        cadr(car(items));
        }
    else
        {
        assoc(a,cdr(items));
        }
    }

function map(f,items)
    {
    if (null?(items),null,cons(f(car(items),map(f,cdr(items)))));
    }

function make-assoc(tags,values)
    {
    if (null?(tags),
        null,
        cons(list(car(tags),car(values)),
                make-assoc(cdr(tags),cdr(values))));
    }

function symbol?(x) { type(x) == :SYMBOL; }
function object?(x) { pair?(x) && car(x) == :object; }
function scope?(x) { pair?(x) && car(x) == :scope; }
function quote?(x) { pair?(x)  && car(x) == :quote; }
function define?(expr) { pair?(expr) && car(expr) == :define; }
function atom?(expr) { not(pair?(expr)); }

function ||(#,a,$)
    {
    //println("or: ",a," ",$);
    while (not(a) && not(null?($)))
        {
        a = eval(car($),#);
        $ =  cdr($);
        }
    a;
    }

function caar(i) { car(car(i)); }
function cadr(i) { car(cdr(i)); }
function cddr(i) { cdr(cdr(i)); }
function caddr(i) { car(cdr(cdr(i))); }

function =(#,$location,value)
    {
    if (pair?($location)&& car($location) == :[)
        {
        setElement!(eval(cadr($location),#),eval(caddr($location),#),value);
        }
    else if (pair?($location)&& car($location) == :.)
        {
        set!(caddr($location),value,eval(cadr($location),#));
        }
    else
        {
        set!($location,value,#);
        }
    }
