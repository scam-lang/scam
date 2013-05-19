function fwhile($test, $body)
    {
    println("forcing test");
    while(force($test))
        {
	println("forcing body");
	force($body);
	}
    }
var i = 0;

println("while as imperative statement");
while (i < 10)
    {
    print("i is ", i, "\n");
    i = i + 1;
    }
println("while as a function call");
while (i > 0,
    {
    i = i - 1;
    print("i is ", i, "\n");
    });
println("user while that returns from global scope");
fwhile (:true)
    {
    println("returning!");
    return 0;
    }

print("you should not be seeing this message\n");
