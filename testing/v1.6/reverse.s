function reverse(str)
    {
    println("reversing ", str);
    if (length(str) == 1)
        {
	str;
	}
    else
	{
        reverse(tail(str)) + head(str);
	}
    }

function reverse2(str)
    {
    function iter(store, src)
        {
	if (src == "")
	    {
	    store;
	    }
	else
	    {
	    iter(head(src) + store, tail(src));
	    }
	}
    iter("", str);
    }

var x = "hello";
var y = reverse(x);
print("reverse(", x, ") is ", reverse(x), "\n");
print("reverse2(", x, ") is ", reverse2(x), "\n");
