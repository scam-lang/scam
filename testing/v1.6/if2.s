var if =
    {
    var old-if = if;
    function new-if(test, $tBranch, $fBranch)
        {
	old-if(test)
	    {
	    force($tBranch);
	    }
	else
	    {
	    force($fBranch);
	    }
	}
    new-if;
    };

if (3 < 4)
    {
    print("three *is* less than 4!\n");
    }
else
    {
    print("oops!\n");
    }
