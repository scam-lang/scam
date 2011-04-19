include("basics");

    function node(item,next) { this; }

    function right(items)
	{
	var slots = items . constructor . parameters;
	var lhs = slots[0];
	var rhs = slots[1];

	if (type(items . (lhs)) == :OBJECT 
	&& items . constructor == items . (lhs) . constructor)
	    {
	    var x = items . (lhs);
	    items . (lhs) = x . (rhs);
	    x . (rhs) = items;
	    right(x);
	    }
	else
	    {
	    items;
	    }
	}

    var items = right(1 node 2 node 3 node :null);

    while (items != :null)
	{
	inspect(items . item);
	items = items . next;
	}
