function square(x) { x * x; }
function wow()
    {
    "wow, wow, wow!";
    }

function c(x)
    {
    function wow() { x; }
    this;
    }
function b(y)
    {
    function yow()
	{
	square(wow());
	}
    proxy(c(y * y));
    }
function a(z)
    {
    function zow()
	{
	square(yow());
	}
    proxy(b(z * z));
    }
var obj = a(2);

println("wow() is ", wow());
println("obj . wow() is ", obj . wow());
println("obj . yow() is ", obj . yow());
println("obj . zow() is ", obj . zow());
