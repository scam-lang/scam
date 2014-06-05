function new-if(test, $then, $else)
    {
    if (test)
        {
	force($then);
	}
    else 
        {
	force($else);
	}
    }

function recur(n, sum)
    {
    function iter(n,sum)
        {
	new-if (n == 0)
	    {
	    sum;
	    }
	else
	    {
	    iter(n - 1,  sum + n);
	    }
	}
    var result = iter(n,sum);
    result;
    }

println(recur(100,0));
