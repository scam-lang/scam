include("basics");

function +(x,y)
    {
    println("local + is ", +);
    println("non-local + is ", context . +);
    println("non-non-local + is ", context . context . +);
    println("shadowed + is ", shadowed(:+));

    (shadowed(:+))(x,y);
    }
var x = 3 + 4;
println("x is ", x);
