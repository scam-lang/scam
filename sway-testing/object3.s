include("basics");

function alpha(x)
    {
    this;
    }
function beta(y)
    {
    extends(alpha(y + y));
    }

var a = alpha(4);
var b = a . constructor(5);
var c = beta(6);
var x;

println("a . x is ",a . x);
inspect(a . x);
pp(a . constructor);
inspect(a . constructor . name);
inspect(b . x);
inspect(b . constructor . name);
inspect(a is :alpha);
inspect(b is :alpha);
inspect(b is :beta);
inspect(c is :beta);
inspect(c is :alpha);
inspect(c is :gamma);
