function function($name,$params,$body)
    {
    println("Intercepted!");
    __function__($name,$params,$body);
    }

function greeting()
    {
    println("hello, world!");
    }

println("about to greet...");
greeting();
