include("fib.s");

function fact(n)
    {
    if (n < 2,n,n * fact(n - 1));
    }

println("fib(5) is ",fib(5));
println("fact(5) is ",fact(5));

