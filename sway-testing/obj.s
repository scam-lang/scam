include("basics");

function x()
    {
    var a = 3;
    this;
    }

var b = x();

if (type(b) == :OBJECT)
    {
    println("x() is an object!");
    }
else
    {
    println("x() is not an object!");
    }


println("b . a is ", b . a, " (should be 3)");
println("b . symbol(\"a\") is ", b . symbol("a"), " (should be a)");
println("b . (symbol(\"a\")) is ", b . (symbol("a")), " (should be 3)");
println(".(b,a) is ", .(b,a), " (should be 3)");
println("b . :a is ", b . :a, " (should be 3)");
println("b . (:a) is ", b . (:a), " (should be 3)");
println(".(b,(:a)) is ", .(b,(:a)), " (should be 3)");

b . a = b . a + 1;
println("b . a is ", b . a, " (should be 4)");
