include("fact.s");

function fib(n)
    {
    if (n < 2)
	{
	n;
	}
    else
	{
	fib(n - 1) + fib(n - 2);
	}
    }
var x = 0;
var result;
var t = time();

x = 26;

opt(fib . code,:<,<);
opt(fib . code,:-,-);
opt(fib . code,:+,+);
opt(fib . code,:if,if);
opt(fib . code,:fib,fib);

result = fib(x);
display("fib(");
display(x);
display(") is ");
display(result);
display("\n");
display(time() - t);
display(" seconds");
display("\n");
