function x()
    {
    var a = 3;

    this;
    }

function y()
    {
    var b = 5;

    proxy(x());
    }

var z = y();
println("z = y(); z . a is ", z . a);
println("(y()) . a is ", (y()) . a);
println(".(y(),a) is ", .(y(),a));
