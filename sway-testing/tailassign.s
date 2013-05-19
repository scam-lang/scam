include("basics");
var a = list(1,2,3,4,5);
var b = a;
println("a is ",a);
a = tail(a);
println("after set a to its tail, a is ",a);
inspect(length(a));
inspect(type(a));
println("b was set to the original a");
inspect(b);
inspect(length(b));
inspect(type(b));
tail(tail(a)) tail= array(11,12,13,14);

while (a != :null)
    {
    inspect(a);
    a = tail(a);
    }
println();
while (b != :null)
    {
    inspect(b);
    b = tail(b);
    }
