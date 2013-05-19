function AND(in1, in2, out, agenda)
    {
    var delay = 4; //milliseconds
    function recalculate()
	{
	if (in1 . get() == 1 && in2 . get() == 1)
	    {
	    agenda . schedule(out . set(1), delay);
	    }
	else
	    {
	    agenda . schedule(out . set(0), delay);
	    }
	}

    in1 . register(this);
    in2 . register(this);
    recalculate();
    this;
    }


function OR(in1, in2, out, agenda)
    {
    var delay = 3; //milliseconds
    function recalculate()
	{
	if (in1 . get() == 1 || in2 . get() == 1)
	    {
	    agenda . schedule(out . set(1), delay);
	    }
	else
	    {
	    agenda . schedule(out . set(0), delay);
	    }
	}

    in1 . register(this);
    in2 . register(this);
    this;
    }


function NOT(in, out, agenda)
    {
    var delay = 1; //milliseconds
    function recalculate()
	{
	if (in . get() == 0)
	    {
	    agenda . schedule(out . set(1), delay);
	    }
	else
	    {
	    //println("output is 0");
	    agenda . schedule(out . set(0), delay);
	    }
	}

    in . register(this);
    this;
    }
