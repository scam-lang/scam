include("sway.s");

function f(x)
    {
    var result;
    println("beginning f(",x,").");
    result = catch(g(2 * x));
    if (type(result) == :error)
	{
	if (result . code == :undefinedVariable)
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

    //throw(:undefinedVariable,"take that!");
    throw(:myError,"take that!");
    result = mnenom * y;
    println("done with g(",y,").");
    return result;
    }
println(f(4));
