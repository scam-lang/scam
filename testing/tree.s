var key = 0;
var left = 1;
var right = 2;
var tree = array(:null,:null,:null);

function printTree(t)
    {
    if (t == :null)
        {
	print("()");
	}
    else
        {
	print("(",t . key, " ");
	printTree(t . left);
	print(" ");
	printTree(t . right);
	print(")");
	}
    }

var insert = function(t, v)
    {
    if (t == :null)
	{
	t = array(:null,:null,:null);
	t . key = v;
	//sway();
	return t;
	}
    else if (v < t . key)
	{
	t . left = insert(t . left, v);
	return t;
	}
    else if (:true)
        {
	t . right = insert(t . right, v);
	return t;
	}
    };

var find = function (t, v)
    {
    if (t == :null || v == t . key)
	{
	t;
	}
    else if (v < t . key)
	{
	find(t . left, v);
	}
    else
	{
	find(t . right, v);
	}
    };

function lookup(t, i)
    {
    var result = find(t, i);

    if (result == :null)
	{
	"not found!";
	}
    else if (result . left == :null && result . right == :null)
	{
	"it's a leaf!";
	}
    else
	{
	"it's an interior node!";
	}
    }

function main()
    {
    var i;
    var num = :null;
    var input = array(1, 2, 3);
    var search = array(4, 2, 3);
    var t = :null;

    i = 0;
    while (i < length(input))
        {
	num = input . i;
        t = insert(t, num);
	print(num," inserted.\n");
	i = i + 1;
	}
    
    print("insertion phase complete, tree is ");
    printTree(t);
    print(".\n");

    i = 0;
    while (i < length(search))
        {
	num = search . i;
	print("looking for ", num, ": ", lookup(t, num), "\n");
	i = i + 1;
	}

    print("good-bye!\n");
    }

print("hello\n");
main();
