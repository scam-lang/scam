var built-in = context;
function while($test,$body,$)
    {
    var result = force($test);
    if (result)
        {
	force($body);
	built-in . while(force($test))
	    {
	    force($body);
	    }
	}
    else if ($ != :null)
        {
	force($ . 0);
	}
    }

var i;

i = 0;
while (i < 5)
    {
    println("i is ", i);
    i = i + 1;
    }

while (i < 5)
    {
    println("i is ", i);
    i = i + 1;
    }
else
    {
    println("i was too large!");
    }

