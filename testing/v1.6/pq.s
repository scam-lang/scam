include("basics");
include("priorityQueue.s");

function main()
    {
    var items = array(3,2,0,1,4);

    var pq = priorityQueue();

    var i = 0;

    while(i < length(items))
        {
	println("enqueueing: ", items . i);
	pq . enqueue(items . i, items . i * 2);
	println("    first item is ", pq . peekItem());
	println("    first rank is ", pq . peekRank());
	i = i + 1;
	}

    println();

    while(!(pq . empty()))
        {
	println("dequeueing: ", pq . dequeue());
	i = i + 1;
	}
    }

main();
