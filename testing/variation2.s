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
	}

    proxy(x());
    }

var xo = x();
var yo = y();

println("(x()) . a() yields...");
(x()) . a();
println("(y()) . a() yields...");
(y()) . a();
