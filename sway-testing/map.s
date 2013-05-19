var null = :null;

function map(f, items)
    {
    print("mapping ");
    printSequence(items);
    print("\n");
    if (items == null)
        {
	print("returning null\n");
	return null;
	}
    else
        {
	var rest = map(f, tail(items));
	print("joining ", f(head(items)), " with ", rest, "\n");
	return join(f(head(items)), rest);
	}
    }

function square(item)
    {
    return item * item;
    }

function printSequence(items)
    {
    print("[ ");
    while (items != null)
        {
	print(head(items), " ");
	items = tail(items);
	}
    print("] ");
    }

var original = array(1,2,3,4);
var modified = map(square, original);

printSequence(original);
print("\n");
printSequence(modified);
print("\n");
