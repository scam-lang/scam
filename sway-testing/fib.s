println("starting fib.s");

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

x = 20;
result = fib(x);
println("fib: fib(",x,") is ",result);
println(time() - t," seconds");
