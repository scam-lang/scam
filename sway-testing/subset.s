function map(f, items)
    {
    if (items == :null)
        {
	return :null;
	}
    else
        {
	return join(f(head(items)), map(f, tail(items)));
	}
    }

function powerSet(set)
    {
    function prepend(s)
        {
	return join(head(set), s);
	}

    if (set == :null)
        {
	return emptySet;
	}
    else
        {
	var rest = powerSet(tail(set));
	return rest + map(prepend, rest);
	}
    }

function printSequence(items)
    {
    print("[ ");
    while (items != :null)
        {
	print(head(items), " ");
	items = tail(items);
	}
    print("] ");
    }

var emptySet = array(:null);
var original = array(1,2,3);

var i = 0;
var size = 0;
var pset = :null;

pset = powerSet(original);
size = length(pset);

i = 0;
while(i < size)
    {
    printSequence(pset . i);
    print("\n");
    i = i + 1;
    }

