include("g.s");

function f(n)
    {
    var result;
    if (n < 2)
        {
	result =  1;
	}
    else
        {
	var rest = f(n - 1);
	result = n * rest;
	}
    return result;
    }

var x = 10;

print("f.s: f(",x,") is ", f(x), "\n");
print("f.s: g(",x,") is ", g(x), "\n");
