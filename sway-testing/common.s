include("sway.lib");

var z-common = { var count = 0; this; };

function z()
    {
    var parent = null;
    var common = z-common;
    var uncommon = { var count = 0; this;};
    this;
    }

function x()
    {
    var parent = z();
    parent . common . count = parent . common . count + 1;
    parent . uncommon . count = parent . uncommon . count + 1;
    this;
    }

function y()
    {
    var parent = z();
    parent . common . count = parent . common . count + 1;
    parent . uncommon . count = parent . uncommon . count + 1;
    this;
    }

var xish = new(x());
inspect(xish . common . count);
var yish = new(y());
inspect(xish . common . count);

println("x's common count is ", xish . common . count);
println("y's common count is ", yish . common . count);
println("x's uncommon count is ", xish . uncommon . count);
println("y's uncommon count is ", yish . uncommon . count);
