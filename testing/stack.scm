function stack()
    {
    var store = :null;

    function push(item)
        {
	store = cons(item,store);
	item;
	}
    function pop()
        {
	var item = car(store);
	store = cdr(store);
	item;
	}
    function empty?()
        {
	store == :null;
	}

    this;
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
    }
