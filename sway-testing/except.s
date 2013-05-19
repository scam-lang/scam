include("sway.s");

function try(#,$a,$b)
    {
    var result = catch(eval($a,#));
    if (type(result) == :error)
        {
        eval($b,#);
        }
    else
        {
        result;
        }
    }

function fact(n)
    {
    if (n == 0)
        {
        try ()
            {
            zzz;
            }
        else
            {
            println("[ERROR]");
            1;
            }
        }
    else
        {
        return n * fact(n - 1);
        }
    }

print("fact(5) is ", fact(5), "\n");
