    function try($store,$code,$alternative)
        {
        var result = catch(force($code));
        if (type(result) == :ERROR)
            {
            $store = result;
            force($alternative);
            }
        result;
        }

    var error;
    var a = 6,b = 0;

    try (error)
        {
        a / b;
        }
    else if (error . type == :undefinedVariable)
        {
        println("try block has an undefined variable");
        }
    else 
        {
        println("try block has a divide error");
        }
