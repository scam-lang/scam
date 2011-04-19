function f(a)
    {
    this;
    }

var obj = f(2);

obj . (:a) = 4;

println("obj . a is ",obj . a);
println("obj . a is ",obj . :a);
println("obj . a is ",obj . (:a));

obj . :a = 4;
