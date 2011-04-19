var mnemon = 10;
function f(x)
    {
    var result;
    println("beginning f(",x,").");
    result = catch(g(2 * x));
    if (type(result) == :ERROR)
	{
	if (result . type == :undefinedVariable)
	    {
	    result = x;
	    }
	else
	    {
	    println("rethrowing the error");
	    throw(result);
	    }
	}
    println("done with f(",x,").");
    return result;
    }
function g(y)
    {
    var result;
    println("beginning g(",y,").");

    throw(:myError,"take that!");
    result = mnenom * y;
    println("done with g(",y,").");
    return result;
    }
println(f(4));
