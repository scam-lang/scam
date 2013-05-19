    function term(a,iv,n)
        {
	function value(x) { a * (x ^ n); }
	function zero?() { a == 0; }
	function one?() { a == 1 && n == 0; }
	function toString()
	    {
	    if (a == 0) { "0"; }
	    else if (n == 0) { "" + a; }
	    else if (n == 1 && a == 1) { "" + iv; }
	    else if (n == 1) { "" + a + iv; }
	    else if (a == 1) { "" + iv + "^" + n; }
	    else { "" + a + iv + "^" + n; }
	    }
	function diff(wrtv)
	    {
	    if (wrtv == iv)
	        {
		term(a * n,iv,n - 1);
		}
	    else
	        {
		term(0,wrtv,0);   //return zero as a term
		}
	    }
	this;
	}


    var t = term(5,:w,3);
    var a = t . diff(:w);   //iv and wrtv match
    var b = t . diff(:x);   //iv and wrtv do not match
     
    inspect(t . toString());
    inspect(a . toString());
    inspect(b . toString());
