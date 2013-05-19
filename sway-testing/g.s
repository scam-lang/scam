function g(n)
    {
    var total = 1;

    while (n > 1)
        {
        total = total * n;
        n = n - 1;
        }
    
    total;
    }

var x = 10;

print("g.s: g(",x,") is ", g(x), "\n");
