function swap($a,$b)
    {
    var temp = force($a);
    $a = force($b);
    $b = temp;
    }

var a = :a;
var b = :b;

swap(a,b);

println("a is ",a," and b is ",b);
