var x-shared =
    {
    var count = 0;
    function +count() { count = count + 1; }
    this;
    };

function x()
    {
    var shared = x-shared;
    var trials = 5;
    function a()
	{
	print(trials, ": x's a");
	if (trials > 0)
	    {
	    println(", calling b");
	    trials = trials - 1;
	    b();
	    }
	else
	    {
	    println();
	    }
	}
    function b()
	{
	print("   x's b, calling a\n");
	a();
	}
    shared . +count();

    this;
    }

function y()
    {
    function a()  //override x's a
	{
	print("y's a, calling b\n");
	b();
	}
    function b()  //override x's b
	{
	print("y's b, calling x's version of a\n");
	a . prior();
	}

    proxy(x());
    trials = trials - 2;
    this;
    }

var xish = x();
var yish = y();

println("number of x objects created: ", xish . shared . count);
xish . a();
println("------------------------------");
yish . a();
