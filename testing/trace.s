include("debug");

function f(x)
    {
    var result = x,y = 0;
    result = x + result;
    if (x > 0)
        {
	var i = 42;
	g();
	}
    result;
    }

function g()
    {
    println("in g...");
    }

f . filter = trace*;
println("f(5) is ",f(5));
