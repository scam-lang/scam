include("inherit.s");

var z = 
    {
    var z_shared = { var shared_count = 0; this; };

    function z()
        {
        var count = 0;
        extend(z_shared);
        count = count + 1;
        shared_count = shared_count + 1;
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

println("x's common count is ", xish . shared_count);
println("y's common count is ", yish . shared_count);
println("z's common count is ", zish . shared_count);
println("x's count is ", xish . count);
println("y's count is ", yish . count);
println("z's count is ", zish . count);
inspect(xish);
