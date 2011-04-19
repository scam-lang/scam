var x =
    {
    var a = 2, b = "hello";
    print("the shared environment was made!\n");
    function()
        {
	var c = 5;
	print("an object was made!\n");
	this;
	};
    };

var y = x();
var z = x();

print("y:(a,b,c) is ", y . a, ",", y . b, ",", y . c, "\n");

y . a = 3;
y . c = 4;

println("a and b should stay the same, c should change");
print("y:(a,b,c) is ", y . a, ",", y . b, ",", y . c, "\n");
print("z:(a,b,c) is ", z . a, ",", z . b, ",", z . c, "\n");
