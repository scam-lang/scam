function x()
    {
    function a()
        {
	println("x:a() -> hello");
	b();
	}
    function b()
        {
	println("x:b() -> goodbye");
	}

    this;
    }

function y()
    {
    function b()
        {
	println("y:b() -> au revoir");
	print("run the old version of b, via prior: ");
	b . prior();
	}

    proxy(x());
    }

var x-obj = x();
var y-obj = y();

println("x-obj . a() yields...");
x-obj . a();
println("y-obj . a() yields...");
y-obj . a();
