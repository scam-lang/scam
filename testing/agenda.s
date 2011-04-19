include("priorityQueue.s");

function agenda()
    {
    var time = 0;
    var actions = priorityQueue();
    function schedule($action, delay) //action is delayed!
	{
	actions . enqueue($action, time + delay);
	}
    function run()
	{
	while (!(actions . empty()))
	    {
	    time = actions . peekRank();
	    force(actions . dequeue()); //action was delayed!
	    }
	time = 0;
	}

    this;
    }
