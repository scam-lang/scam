include("basics");
function =($a,b)
    {
    var <- = shadowed(:=);
    if (type($a . code) != :VARIABLE)
       {
       throw(:restrictedAssignment,
           "you are only allowed to assign to simple variables");
       }
    $a <- b;
    }

function f(z)
    {
    function x(@)
        {
        if (length(@) == 0)
            {
            z;
            }
        else
            {
            z = @ . 0;
            }
        }
    this;
    }

var o = f(4);
println(o . x());
println(o . x(13));
println(o . x());
o . z = 55;
println(o . x());

