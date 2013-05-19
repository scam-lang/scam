include("inherit.s");

var z = 
    {
    var z-shared = { var shared-count = 0; this; };

    function z()
        {
        var count = 0;
        extend(z-shared);
        this;
        }
    };

function x()
    {
    extend(z());
    }

function y()
    {
    extend(z());
    }

var xish = x();
var yish = y();
var zish = z();

yish . count = 1;
zish . count = 2;

yish . shared-count = 11;
zish . shared-count = 22;

println("x's count is ", xish . count);
println("y's count is ", yish . count);
println("z's count is ", zish . count);
println("x's common count is ", xish . shared-count);
println("y's common count is ", yish . shared-count);
println("z's common count is ", zish . shared-count);
