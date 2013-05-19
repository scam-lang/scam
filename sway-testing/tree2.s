function tree()
    {
    var key = :null;
    var left = :null;
    var right = :null;
    function insert(v)
	{
	if (key == :null)
	    {
	    key = v;
	    }
	else if (v < key && left != :null)
	    {
	    left . insert(v);
	    }
	else if (v < key)
	    {
	    left = tree();
	    left . insert(v);
	    }
	else if (right != :null)
	    {
	    right . insert(v);
	    }
	else
	    {
	    right = tree();
	    right . insert(v);
	    }
	}
    function find(v)
	{
	if (key == :null)
	    {
	    this;
	    }
	else if (v == key)
	    {
	    this;
	    }
	else if (v < key && left != :null)
	    {
	    left . find(v);
	    }
	else if (v < key)
	    {
	    :null;
	    }
	else if (right != :null)
	    {
	    right . find(v);
	    }
	else
	    {
	    :null;
	    }
	}
    this;
    }

function lookup(t,i)
    {
    var result = t . find(i);

    if (result == :null)
	{
	"not found!";
	}
    else if (t . key == :null)
	{
	"empty tree";
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
    var size = 3;
    var t = tree();

    i = 0;
    while (i < size)
        {
	print("inserting ", data . i, "\n");
        t . insert(data . i);
	i = i + 1;
	}
    
    i = 0;
    while (i < size)
        {
	print("looking for ", search . i, ": ", lookup(t,search . i),"\n");
	i = i + 1;
	}

    print("good-bye!\n");
    }

main();
