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
    proxy(x());
    b = function()
        {
	println("y:b() -> au revoir");
	};

    this;
    }

println("(x()) . a() yields...");
(x()) . a();
println("(y()) . a() yields...");
(y()) . a();
