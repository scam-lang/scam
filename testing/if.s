//overload readInt to get rid of interactivity
function readInt()
    {
    3;
    }

var x = 2;

println("enter 0, 1, or 2: ");

x = readInt();

println("you entered ", x);

if (x == 0)
    {
    print("zero\n");
    }
else if (x == 1)
    {
    print("one\n");
    }
else if (x == 2)
    {
    print("two\n");
    }
else
    {
    print("can't follow directions, can you?\n");
    }

