function a()
    {
    var x = 0;
    function display() { println("a: x is ",x); }
    this;
    }
function b()
    {
    var x = 1;
    function display() { println("b: x is ",x); }
    proxy(a());
    }

var aish = a();
var bish = b();

aish . display();
bish . display();
bish . display . prior();
