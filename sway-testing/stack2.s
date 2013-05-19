include("basics");
include("stack.s");

function stack()
    {
    function pop()
        {
	if (empty?())
	    {
	    throw(:stackError,"pop on empty stack");
	    }
	pop . prior();
	}
    proxy((shadowed(:stack))());
    }

if (:true)
    {
    var s = stack();

    s . push(:hello);
    s . push("world");
    s . push(42);

    while (!(s . empty?()))
        {
	println(s . pop());
	}

    println(s . pop());
    }
