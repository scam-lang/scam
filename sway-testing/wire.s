function wire()
    {
    var value = 0;  //initial (nonsensical) value
    var downstream = :null;
    function register(object)
	{
	downstream = object join downstream;
	object . recalculate();
	}
    function inform(object)
	{
	object . recalculate();
	}
    function set(newValue)
	{
	if (newValue != value)
	    {
	    value = newValue;
	    map(inform, downstream);
	    }
	}
    function get()
	{
	return(value);
	}

    this;
    }

