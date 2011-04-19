function f()
    {
    g() + 0;
    }

function g()
    {
    var error;
    error = catch(h());
    println("rethrowing error");
    throw(error);
    }

function h()
    {
    throw(:ouch,"hey!");
    2;
    }

f() + 0;
:ok;
