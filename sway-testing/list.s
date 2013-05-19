function cons(a,b)
    {
    return array(a,b);
    }

function car(x)
    {
    x . 0;
    }
function cdr(x)
    {
    x . 1;
    }

function list(@)
    {
    if (@ == :null)
        {
        return :null;
        }
    else
        {
        return cons(head(@),apply(list,tail(@)));
        }
    }

function map(f, x)
    {
    if (x == :null)
        {
        return :null;
        }
    else
        {
        return cons(f(car(x)),map(f,cdr(x)));
        }
    }

function append(a,b)
    {
    if (a == :null)
        {
        b;
        }
    else
        {
        return cons(car(a),append(cdr(a),b));
        }
    }

function pset(s)
    {
    function prepend(x)
        {
        cons(car(s),x);
        }

    if (s == :null)
        {
        return cons(:null,:null);
        }
    else
        {
        var rest = pset(cdr(s));
        return append(rest, map(prepend, rest));
        }
    }

function
printList(pre, a, post)
    {
    var i = 0;

    if (pre != :null) {print(pre);}
    print("[ ");
    while (a != :null)
        {
        print(car(a), " ");
        a = cdr(a);
        }
    print("]");
    if (post != :null) {print(post);}
    }

//printList(:null, append(list(2,3,4),list(1,2,3)), "\ndone\n");
//printList(:null, map(function(x){ x + 1; }, list(1,2,3)), "\ndone\n");

var set = list(1,2,3,4);

set = pset(set);

while (set != :null)
    {
    printList(:null, car(set), "\n");
    set = cdr(set);
    }
