function priorityQueue()
    {
    var items;
    function bundle(data,rank,next) { return this; }
    function dequeue()
	{
	var item;
	
	item = items . next . data;
	items . next = items . next . next;
	return item;
	}
    function enqueue(item,rank)
	{
	var prefix = items;
	var suffix = items . next;
	var package = bundle(item,rank,:null);

	while (suffix != :null && rank >= suffix . rank)
	    {
	    prefix = suffix;
	    suffix = suffix . next;
	    }

	prefix . next = package;
	package . next = suffix;
	}
    function peekItem() { return items . next . data; }
    function peekRank() { return items . next . rank; }
    function empty()    { return items . next == :null; }

    items = bundle(:null,:null,:null); //dummy head node
    this;
    }
