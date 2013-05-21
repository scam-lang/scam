include("sway.lib");

function try(#,$error,$a,$b)
    {
    var result = catch(eval($a,#));
    if (type(result) == 'error)
        {
        set($error,result,#);
        result = eval($b,#);
        }
    result;
    }

function normalize(a,b)
    {
    return (a + b) * (a + b);
    }

function g()
    {
    var result;
    var error;

    //println("in g...");
    try(error) 
        {
        throw('hiy,"ouch"); // comment this line out and a 2 should result
        result = normalize(1,2);
        }
    else if (error . code == 'nonFunction)
        {
        result = 2;
        }
    else if (error . code == 'hey)
        {
        result = 100;
        }
    else if (error . code == 'hay)
        {
        result = 33;
        }
    else
        {
        throw(error);
        }
    result;
    }

var + = 3;
println(g());
