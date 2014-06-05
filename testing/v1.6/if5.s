var built-in = context;

function if(test,$ifTrue,$ifFalse)
    {
    built-in . if(test)
        {
	force($ifTrue);
	}
    else
        {
	force($ifFalse);
	}
    }

if (3 < 4)
    {
    println("3 is less than 4!");
    }
else
    {
    println("oops.");
    }
