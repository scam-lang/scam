include("sway.lib");

var methods = array(symbol("-"),'+,symbol("++"),symbol("+-"),'-+);
inspect(length(methods));
inspect(methods);

var i = 0;

while (i < length(methods))
    {
    if (defined?(methods[i],this))
        {
        println(methods[i], " is defined!");
        }
    else
        {
        println(getElement(methods,i), " is not defined");
        }
    i = i + 1;
    }
