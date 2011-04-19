function try($error,$a,$b)
    {
    var result = catch(force($a));
    if (type(result) == :ERROR)
	{
	$error = result;
        force($b);
	}
    else
	{
        result;
	}
    }

function normalize(a,b)
    {
    return (a + b) * (a + b);
    }

function g()
    {
    var result;
    var error;

    try(error) 
        {
	throw(:hiy,"ouch"); // comment this line out and a 2 should result
	result = normalize(1,2);
	}
    else if (error . type == :nonFunction)
        {
	result = 2;
	}
    else if (error . type == :hey)
	{
	result = 100;
	}
    else if (error . type == :hay)
        {
	result = 33;
	}
    else
        {
	throw(error);
	}
    result;
    }

var + = 3;
println(g());
