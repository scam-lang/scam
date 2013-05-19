include("basics");

function with(obj,$actions)
    {
    $actions . context = obj;
    force($actions);
    }


function f(x) { var y = x + 1; this;}

var o = f(3);

inspect(o . x);
inspect(o . y);

with (o)
    {
    x = 10;
    y = 13;
    }

inspect(o . x);
inspect(o . y);
