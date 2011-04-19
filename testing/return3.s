include("basics");

var i = 3;
var j = 8;

function even?(x) { x % 2 == 0; }

function f()
    {
    var i = 3,j = 2;
    var r = catch(return i * j);

    println("r is ",r);
    for (i = 0, i < 5, i = i + 1)
	{
	for (j = 1, j < 5, j = j + 1)
	    {
	    println("i is ",i," and j is ",j);
	    if (even?(j) && j == i)
		{
		println("trying to leave f...");
		throw(r);
		}
	    }
	}
    }

function g()
    {
    var z;
    println("g executing...");
    z = f();
    println("g's z is ", z);
    println("g done.");
    z;
    }

println(g());
println("done");

