include("debug");

var root = :null;

function node(value,left,right)
    {
    var parent;
    var color;

    function display()
        {
	println("value:  ", value);
	println("left:   ", left);
	println("right:  ", right);
	println("parent: ", parent);
	println("color:  ", color);
	}

    this;
    }

function printTree(t)
    {
    function iter(r, indent)
        {
	if (r == :null)
	    {
	    println("null");
	    }
	else
	    {
	    println(r . value, "(", r . color, ")");
	    print(indent, "left:  ");
	    iter(r . left,indent + "    ");
	    print(indent, "right: ");
	    iter(r . right,indent + "    ");
	    }
	}

    iter(t, "   ");
    }

function insert(t, v, op)
    {
    if (t == :null)
	{
	root = node(v,:null,:null);
	root . parent = root;
	insertionFixup(root);
	}
    else
        {
	var lessThan = v op t . value;

	if (lessThan && t . left != :null)
	    {
	    insert(t . left, v, op);
	    }
	else if (lessThan)
	    {
	    t . left = node(v,:null,:null);
	    t . left . parent = t;
	    insertionFixup(t . left);
	    }
	else if (t . right != :null)
	    {
	    insert(t . right, v, op);
	    }
	else
	    {
	    t . right = node(v, :null, :null);
	    t . right . parent = t;
	    insertionFixup(t . right);
	    }
	}
    }

function prune(x)
    {
    assert(leaf?(x) == :true);
    if (leftChild?(x))
        {
	parent(x) . left = :null;
	}
    else if (rightChild?(x))
        {
	parent(x) . right = :null;
	}
    else
        {
	root = :null;
	}
    }

function swapToLeaf(x)
    {
    if (leaf?(x) == :false)
        {
	var y;
	var temp;

	if (x . right != :null)
	    {
	    y = findMin(x . right);
	    }
	else
	    {
	    y = findMax(x . left);
	    }

	temp = x . value;
	x . value = y . value;
	y . value = temp;

	swapToLeaf(y);
	}
    else
        {
	x;
	}
    }
function findMin(x)
    {
    while (x . left != :null)
        {
	x = x . left;
	}
    return x;
    }
function findMax(x)
    {
    while (x . right != :null)
        {
	x = x . right;
	}
    return x;
    }
function delete(x)
    {
    x = swapToLeaf(x);
    deletionFixup(x);
    //println("pruning ", x . value);
    prune(x);
    }

function deletionFixup(x)
    {
    while (root?(x) == :false && x . color == :black)
	{
	if (red?(sibling(x)))
	    {
	    parent(x) . color = :red;
	    sibling(x) . color = :black;
	    rotate(sibling(x),parent(x));
	    // should have black sibling now
	    assert(sibling(x) . color == :black);
	    }
	else if (red?(nephew(x)))
	    {
	    sibling(x) . color = parent(x) . color;
	    parent(x) . color = :black;
	    nephew(x) . color = :black;
	    rotate(sibling(x),parent(x));
	    x = root;
	    // subtree is bh balanced
	    // with proper bh contribution
	    }
	else if (red?(niece(x)))
	    {
	    // nephew must be black
	    niece(x) = :black;
	    sibling(x) = :red;
	    rotate(neice(x),sibling(x));
	    // should have red nephew now
	    assert(nephew(x) . color == :red);
	    }
	else
	    {
	    // sibling, niece, and nephew must be black
	    sibling(x) . color = :red;
	    x = parent(x);
	    // subtree is bh balanced
	    // but has deficit in bh contribution
	    }
	}

    x . color = :black;
    }

function insertionFixup(x)
    {
    x . color = :red;

    while (root?(x) == :false && x . parent . color == :red)
	{
	if (red?(uncle(x)))
	    {
	    parent(x) . color = :black;
	    uncle(x) . color = :black;
	    grandparent(x) . color = :red;
	    x = grandparent(x);
	    }
	else
	    {
	    // uncle must be black

	    if (linear?(x, parent(x), grandparent(x)) == :false)
		{
		var oldParent = parent(x);
		rotate(x,parent(x));
		x = oldParent;
		}

	    parent(x) . color = :black;
	    assert(x . parent . color == :black);
	    grandparent(x) . color = :red;
	    rotate(parent(x),grandparent(x));
	    }
	}

    root . color = :black;
    }

function root?(x) { x == x . parent; }
function leftChild?(x) { return parent(x) . left == x; }
function rightChild?(x) { return parent(x) . right == x; }
function leaf?(x) { x . left == :null && x . right == :null; }
function red?(x) { return x != :null && x . color == :red; }
function black?(x) { return x == :null || x . color == :black; }

function sibling(x)
    {
    if (leftChild?(x))
	{
	x . parent . right;
	}
    else if (rightChild?(x))
        {
	x . parent . left;
	}
    else
	{
        :null;
	}
    }
function niece(x)	//precondition: sibling exists
    {
    if (leftChild?(x))
        {
        sibling(x) . left;
        }
    else
        {
	sibling(x) . right;
	}
    }
function nephew(x)	//precondition: sibling exists
    {
    if (leftChild?(x))
        {
        sibling(x) . right;
        }
    else
        {
	sibling(x) . left;
	}
    }
function parent(x) { x . parent; }
function grandparent(x) { parent(x) . parent; }
function uncle(x)
    {
    if (leftChild?(parent(x)))
        {
	return grandparent(x) . right;
	}
    else if (rightChild?(parent(x)))
        {
	return grandparent(x) . left;
	}
    else
        {
	return :null;
	}
    }
function linear?(x,y,z)
    {
    return
        ((leftChild?(x)  && leftChild?(y)) ||
	 (rightChild?(x) && rightChild?(y)));
    }


function rotate(x,p)
    {
    if (p . left == x) 
        {
	//rotate right
	rotator(x,p,:right,:left);
	}
    else if (p . right == x)
        {
	//rotate left
	rotator(x,p,:left,:right);
	}
    else
        {
	throw("rotate error");
	}
    }
function rotator(x,p,direction,oppositeDirection)
    {
    var gp = parent(p);
    var beta = x . (direction);

    p . (oppositeDirection) = beta;
    if (beta != :null) { beta . parent = p; }

    x . (direction) = p;
    p . parent = x;

    if (p == gp)
        {
	root = x;
	x . parent = x;
	}
    else {
        if (gp . (direction) == p) { gp . (direction) = x; }
	else { gp . (oppositeDirection) = x; }
	x . parent = gp;
	}
    }

function findNode(t, v, op)
    {
    if (t == :null || v == t . value)
	{
	t;
	}
    else if (v op t . value)
	{
	findNode(t . left, v, op);
	}
    else
	{
	findNode(t . right, v, op);
	}
    }

function main()
    {
    var i;
    var num = :null;
    var x;
    var input = array(1, 2, 9, 3, 6, 4, 7, 8, 5);

    i = 0;
    while (i < length(input))
        {
	num = input . i;
	println("inserting!");
        insert(root, num, <);
	print(num," inserted.\n");
	printTree(root);
	i = i + 1;
	}
    
    println("insertion phase complete, tree is...");
    printTree(root);
    println("deletion phase begins...");

    i = 0;
    while (i < length(input))
        {
	num = input . i;
	x = findNode(root,num,<);
	delete(x);
	print(num," deleted.\n");
	printTree(root);
	i = i + 1;
	}

    println("deletion phase complete, tree is...");
    print("good-bye!\n");
    }

print("hello\n");
main();
