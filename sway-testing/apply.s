//include("inherit.s");

function f(@)
    {
    if (@ == null)
        {
        0;
        }
    else
        {
        inspect(@);
        car(@) + apply(f,tail(@));
        }
    }

function g(#,$)
    {
    var total = 0, size = length($);

    while (size > 0)
        {
        total = total + eval(getElement($,(size - 1)),#);
        size = size - 1;
        }

    return total;
    }

print("sum (via f): ",f(1,2,3,4),"\n");
print("sum (via g): ",g(1,2,3,4),"\n");
