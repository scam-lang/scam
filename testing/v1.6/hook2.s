function var($name,$value)
    {
    println("intercepted! initializer is ",$value . code);
    __var__($name,force($value));
    }

function greeting()
    {
    var msg = "hello, world!";
    println(msg);
    }

println("about to greet...");
greeting();
