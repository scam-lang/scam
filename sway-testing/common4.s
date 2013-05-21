include("sway.lib");

function helper() { println("oops!"); }

var common = 10;

function z()
    {
    var parent = null;
    function getCommon() { helper(); common; }
    common = common + 1;
    this;
    }

function x()
    {
    var parent = z();
    var common = 1;
    function helper() { println("help!"); }
    this;
    }

var z1 = z();
inspect(z1);
var z2 = new(z());
inspect(z2);

inspect(z2 . getCommon());

println("making xish...");
var xish = new(x());

inspect(new(z()) . getCommon());
inspect(new(x()) . getCommon());
inspect(new(x()) . getCommon());
inspect(new(z()) . getCommon());
