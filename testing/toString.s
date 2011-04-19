include("basics");
include("objects");

function f(x)
    {
    function toString()
        {
	"<f x:" + string(x) + ">";
	}
    this;
    }

var fo = f(20);
var fa = array(fo,fo);
var a = array(f(1),f(2),f(fo),f(fa));

println("list(1,2) is ",list(1,2));
println("----------------");
inspect("array" + string(array(1,2,3)));
println("----------------");
println("fo is ",fo);
println("----------------");
println("fa is ",fa);
println("----------------");
println("a is ",a);
println("----------------");
inspect(map(string,a));
println("----------------");
println("" + string(a));
println("----------------");
map(function (i) {println("map: ",i);},a);
println("----------------");
