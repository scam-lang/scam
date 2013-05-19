include("inherit.s");

function f(@)
    {
    if (@ == :null)
        {
        0;
        }
    else
        {
        @ . 0 + apply(f,tail(@));
        }
    }

function g($)
    {
    var total = 0, size = length($);

    while (size > 0)
        {
	total = total + force($ . (size - 1));
	size = size - 1;
	}

    return total;
    }

print("sum (via f): ",f(1,2,3,4),"\n");
print("sum (via g): ",g(1,2,3,4),"\n");
