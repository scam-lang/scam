function fwhile($test, $body)
    {
    while(force($test))
        {
	force($body);
	}
    }

function f(x)
    {
    if (x == 0)
        {
	while (:true)
	    {
	    println("in while, returning!");
	    return 0;
	    println("after return in while [SHOULD NOT PRINT]");
	    }
	}
    else
        {
	fwhile (:true)
	    {
	    println("in fwhile, returning!");
	    return 0;
	    println("after return in fwhile [SHOULD NOT PRINT]");
	    }
	}
    println("done with f(x) [SHOULD NOT PRINT]");
    }

function g()
    {
    function r()
        {
	println("    in function r");
	return 0;
	}
    println("    calling function r()");
    r();
    println("    done calling function r()");
    return 1;
    }

println("before calling f(0)");
f(0);
println("after calling f(0)");
println("before calling f(1)");
f(1);
println("after calling f(1)");
println("before calling g()");
g();
println("after calling g()");
