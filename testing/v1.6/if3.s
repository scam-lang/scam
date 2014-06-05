var x = 0;
var built-in = context;

function if(test, $tBranch, $)
    {
    built-in . if(length($) > 1)
        {
	throw(:argumentCountError,"too many arguments to if");
	}

    print("the test is ", test, "\n");
    built-in . if(test)
	{
	force($tBranch);
	}
    else built-in . if(length($) == 1)
	{
	force($ . 0);
	}
    }

while (x < 4)
    {
    if (x == 0)
	{
	print("naughts!\n");
	}
    else if (x == 1)
	{
	print("onesies!\n");
	}
    else if (x == 2)
	{
	println("binary!");
	}
    else if (x == 3)
	{
	println("trinary!");
	}
    x = x + 1;
    }

println("call if with too many arguments");

if (:true,1,1,1);
