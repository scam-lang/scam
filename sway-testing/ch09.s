function variable(name)
    {
	function value(x) { x; }
	function toString() { "" + name; }
	function diff(wrtv)
	    {
	    if (wrtv == name)
	        {
		term(1,wrtv,0);   //really, the number one
		}
	    else
	        {
		term(0,wrtv,0);   //really, the number zero
		}
	    }
	this;
	}

function term(a,iv,n)
    {
	function value(x) { a * (x ^ n); }
	function zero?() { a == 0; }
	function one?() { a == 1 && n == 0; }
	function ivstr()
	    {
	    if (iv is? 'variable)
	        {
		iv . toString();
		}
	    else
	        {
		"(" + iv . toString() + ")";
		}
	    }
	function toString()
	    {
	    if (a == 0) { "0"; }
	    else if (n == 0) { "" + a; }
	    else if (n == 1 && a == 1) { "" + iv . toString(); }
	    else if (n == 1) { "" + a + ivstr(); }
	    else if (a == 1) { "" + ivstr() + "^" + n; }
	    else { "" + a + ivstr() + "^" + n; }
	    }
	function diff(wrtv)
	    {
	    term(a * n,iv,n - 1) times iv . diff(wrtv);
	    }
	if (type(iv) == 'SYMBOL) { iv = variable(iv); }
	this;
	}

    function plus(p,q)
	{
	function value(x) { p . value(x) + q . value(x); }
	function zero?() { p . zero?() && q . zero?(); }
	function one?()
	    {
	    (p . zero?() && q . one?()) || (p . one?() && q . zero?());
	    }
	function toString() { p . toString() + " + " + q . toString(); }
	function diff(wrtv)
	    {
	    var dp/dx = p . diff(wrtv);
	    var dq/dx = q . diff(wrtv);
	    plus(dp/dx,dq/dx);
	    }
	if (p . zero?()) { q; }
	else if (q . zero?()) { p; }
	else { this; }
	}

    function times(p,q)
	{
	function value(x) { p . value(x) * q . value(x); }
	function toString() { p . toString() + " * " + q . toString(); }
	function diff(wrtv)
	    {
	    var dp/dx = p . diff(wrtv);
	    var dq/dx = q . diff(wrtv);
	    (p times dq/dx) plus (q times dp/dx);
	    }
	if (p . zero?()) { p; }
	else if (q . zero?()) { q; }
	else if (p . one?()) { q; }
	else if (q . one?()) { p; }
	else { this; }
	}

var t = term(4,'x,3);
var tp = t . diff('x);

inspect(t . toString());
inspect(tp . toString());

var s = term(1,'x,2) plus term(17,'x,0);

inspect(s . toString());

var w = term(3,s,2);
var wp = w . diff('x);

inspect(w . toString());
inspect(wp . toString());
