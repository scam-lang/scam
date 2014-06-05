var n = 400;

function recur1(m)
    {
    if (m == 0)
        {
	println("bottom");
	}
    else
        {
	recur2(m - 1);
	}
    }

function recur2(x)
    {
    println(x);
    recur1(x);
    }

recur1(n);
