//var catch = 5; // comment this line out and an error is printed
   
function f()
    {
    catch(a);
    }

var result = f();
										if (type(result) == :ERROR)
    {
    println("f() returns \"",
        file(result),",line ",line(result),": ",
	result . value,"\"");
    }
else
    {
    println("f() returns ", result);
    }
