include("reflection.s");

var methods = array(symbol("-"),:+,symbol("++"));
inspect(length(methods));
inspect(methods);

var i = 0;

while (i < length(methods))
    {
    if (defined?(getElement(methods,i),this))
        {
        println(getElement(methods,i), " is defined!");
        }
    else
        {
        println(getElement(methods,i), " is not defined");
        }
    i = i + 1;
    }
