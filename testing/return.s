function f()
    {
    var x;
    print("entering f\n");
    x = {
        print("entering block\n");
        return 3;
        print("leaving block (this should not be displayed)\n");
	5;
        };
    print("leaving f (this should not be displayed)\n");
    return 10;
    }

print("f() should return 3\n");
print("f() returns ",f(),"\n");
