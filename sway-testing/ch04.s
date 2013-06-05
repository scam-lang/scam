/* from ch03 */

function ratio(w,dw,h)
    {
    var dh = h(w + dw) - h(w);
    dh / dw;
    }

/* new stuff */

function y(x)
    {
    2 * (x ^ 3);
    }

function dy/dx(x)
    {
    6 * (x ^ 2);
    }

 function test(x)
     {
     var fraction = 1.0 / 1000000.0;
     var dx = x * fraction;
 println("for x = ",x);
     inspect(ratio(x,dx,y));
     inspect(dy/dx(x));
 println();
     }

function term(a,n)
   {
   function value(x)
       {
       a * (x ^ n);
       }
    this;
    }

function powerRule(p)
    {
    var a = p . a;
    var n = p . n;
    term(a * n,n - 1);
    }

var p = term(2,3);
var dp/dx = powerRule(p);

test(10);
test(20);
test(30);

inspect(p . value(4));
inspect(dp/dx . value(4));

