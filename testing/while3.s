function fwhile($test, $body)
    {
    while(force($test))
        {
	force($body);
	}
    }
function f(x)
    {
    fwhile (i < 10)
	{
	j = 0;
	fwhile (j < 10)
	    {
	    print(i, " * ", j, " is ", i * j, "\n");
	    if (i * j == x)
		{
		return 0;
		}
	    j = j + 1;
	    }
	i = i + 1;
	}
    }

var i = 0;
var j = 0;
var stop = 9;

print("run through i and j, stopping when i * j is ", stop, "\n");

f(stop);

print("done\n");
