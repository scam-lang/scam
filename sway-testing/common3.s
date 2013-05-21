include("sway.lib");

var common = 0;

function z()
    {
    var parent = null;
    var uncommon = 0;
    common += 1;
    uncommon += 1;
    this;
    }

function x()
    {
    var parent = z();
    this;
    }

inspect(new(x()) . common);
inspect(new(x()) . uncommon);
inspect(new(x()) . common);
