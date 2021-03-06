function tree()
    {
    var key = :null;
    var left = :null;
    var right = :null;
    this;
    }

var insert = function (t, v)
    {
    if (t == :null)
	{
	t = tree();
	t . key = v;
	return t;
	}
    else if (v < t . key)
	{
	t . left = insert(t . left, v);
	return t;
	}
    else
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
    var data = array(2,3,1);
    var search = array(4,2,1);
    var t = :null;

    i = 0;
    while (i < length(data))
        {
	print("inserting ", data . i, "\n");
        t = insert(t, data . i);
	i = i + 1;
	}
    
    i = 0;
    while (i < length(search))
        {
	print("looking for ", search . i, ": ", lookup(t,search . i),"\n");
	i = i + 1;
	}

    print("good-bye!\n");
    }

main();
