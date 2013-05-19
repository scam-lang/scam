var count = 0;

function x(a)
    {
    function g(y)
        {
	return integer(a + count * y / (sqrt(count) + 1)) % 2;
	}
    this;
    }

function y(z)
    {
    var proxy = new(x,2 * z);
    var i;
    
    for (i = 0, i < z, i = i + 1)
        {
	var obj = x(i * z + a);
	if (obj . g(z * z) == 0,count = count + 1);
	if (g(z * z) == 0,count = count - 1);
	}

    this;
    }

function q(i)
    {
    var error = catch(3 / i);
    if (error . type == :DivideByZero)
        {
	throw(:oops,"zero");
	}
    else
        {
	throw(:spoo,padRight("not zero",random(10)));
	}
    }

function f()
    {
    var i;
    for (i = 0, i < 10000, i = i + 3)
        {
	var l = length(try(g(i),error . value));
	y(l);
	}
    }

f();
inspect(count);
