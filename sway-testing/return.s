function f()
    {
    var x;
    print("entering f\n");
    if (#t)
        {
        print("entering block\n");
        //return 3;
        print("leaving block (this should not be displayed)\n");
        5;
        }
    print("leaving f (this should not be displayed)\n");
    //return 22;
    print("really leaving f (this should not be displayed)\n");
    return 10;
    }

var z = 0;
print("f() should return 3\n");
z = f();
print("f() returns ",z,"\n");
println("done");
