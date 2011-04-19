include("inherit.s");

function helper() { println("oops!"); }

var common = 10;

function z()
    {
    var parent = null;
    function getCommon() { helper(); common; }
    common =  common + 1;
    this;
    }

function x()
    {
    var parent = z();
    var common = 1;
    function helper() { println("help!"); }
    this;
    }

var xish = new(x());

inspect(new(x()) . getCommon());
inspect(new(x()) . getCommon());
inspect(new(z()) . getCommon());
inspect(new(z()) . getCommon());
