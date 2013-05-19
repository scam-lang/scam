include("sway.s");

function define-function (name,params,body,env)
    {
    var donor = lambda () { 1; };
    donor . name = name;
    donor . parameters = params;
    donor . code = body;
    addSymbol(name,donor,env);
    }

function loadAndgo(#,$name,params,values,$body)
    {
    var f = define-function ($name,params,$body,#);
    apply(f,values);
    }

println("first load and go...");
loadAndgo(countdown,list(:a),list(4))
    {
    inspect(a);
    if (a > 0)
        {
        countdown(a - 1);
        }
    }

println("second load and go...");
loadAndgo(countdown,list(:b),list(3))
    {
    inspect(b);
    if (b > 0)
        {
        countdown(b - 1);
        }
    }

println("calling define-function directly...");
(define-function (:countdown,list(:a),
    :{
    inspect(a);
    if (a > 0)
        {
        countdown(a - 1);
        }
    },this))(4);
