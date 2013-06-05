function term(a,n)
    {
    function value(x) { if(a == 0,0,a * (x ^ n)); }
    function toString() { "" + string(a) + "x^" + string(n); }
    function diff()
        {
        term(a * n,n - 1);
        }
    this;
    }

var q = term(2,3);
inspect(q . toString());

function plus(p,q)
    {
    function value(x) { p . value(x) + q . value(x); }
    function toString() { p . toString() + " + " + q . toString(); }
    function diff()
        {
        var dp/dx = p . diff();
        var dq/dx = q . diff();
        plus(dp/dx,dq/dx);
        }
    this;
    }

function line(m,b)
    {
    plus(term(m,1),term(b,0));
    }

var f = line(3,-5);
var fp = f . diff();
var fpp = fp . diff();

inspect(f . p . toString());
inspect(f . toString());
inspect(f is? 'plus);
inspect(f . p . value(0));
inspect(f . q . value(0));
inspect(f . value(0));
inspect(fp . value(0));
inspect(fp . toString());
inspect(fp . value(5));

inspect(fpp . toString());
inspect(fpp . value(0));
inspect(fpp . value(5));

function term(a,n)
    {
    function value(x) { if(a == 0,0,a * (x ^ n)); }
    function diff() { term(a * n,n - 1); }
    function zero?() { a == 0; }
    function one?() { a == 1 && n == 0; }
    function toString()
        {
        if (a == 0) { "0"; }
        else if (n == 0) { "" + string(a); }
        else if (n == 1 && a == 1) { "x"; }
        else if (n == 1) { "" + string(a) + "x"; }
        else if (a == 1) { "x^" + string(n); }
        else { "" + string(a) + "x^" + string(n); }
        }
this;
}

var f = line(3,-5);
var fp = f . diff();
var fpp = fp . diff();

println("-------------------------------------");
inspect(f . toString());
inspect(fp . toString());
inspect(fpp . toString());
inspect(fpp . value(0));
inspect(fpp . value(5));

function sum(p,q)
    {
    function value(x) { p . value(x) + q . value(x); }
    function toString() { p . toString() + " + " + q . toString(); }
    function diff()
        {
        var dp/dx = p . diff();
        var dq/dx = q . diff();
        sum(dp/dx,dq/dx);
        }
    function zero?() { p . zero?() && q . zero?(); }
    function one?()
        {
        (p . zero?() && q . one?()) || (p . one?() && q . zero?());
        }
    if (p . zero?()) { q; }
    else if (q . zero?()) { p; }
    else { this; }
    }

var f = line(3,-5);
var fp = f . diff();
var fpp = fp . diff();

println("-------------------------------------");
inspect(f . toString());
inspect(fp . toString());
inspect(fpp . toString());
inspect(fpp . value(0));
inspect(fpp . value(5));

var z = term(4,3) sum term(3,2) sum term(2,1) sum term(1,0);
var zp = z . diff();
var zpp = zp . diff();
var zppp = zpp . diff();

println("-------------------------------------");
inspect(z . toString());
inspect(zp . toString());
inspect(zpp . toString());
inspect(zppp . toString());
inspect(zpp . value(1));
inspect(zpp . value(2));
inspect(zppp . value(1));
inspect(zppp . value(2));
