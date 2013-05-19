include("sway.s");
include("compile.s");

function fib(n)
    {
    if (n < 2,n,fib(n - 1) + fib(n - 2));
    }

inspect(fib . code);

var x = 0;
var result;
var t = time();

x = 26;

result = fib(x);
println("uncompiled fib: fib(",x,") is ",result);
println(time() - t," seconds");

//compile the fib function

compile(fib,this);

inspect(fib . code);

t = time();

result = fib(x);
println("compiled fib: fib(",x,") is ",result);
println(time() - t," seconds");
