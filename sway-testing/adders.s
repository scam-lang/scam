include("basics");
include("wire.s");
include("logic.s");
include("agenda.s");

function half-adder(a, b, sum, carry, simulator)
    {
    var or->and = wire();
    var not->and = wire();

    AND(a,b,carry,simulator);
    OR(a,b,or->and,simulator);
    NOT(carry,not->and,simulator);
    AND(or->and,not->and,sum,simulator);
    }

function full-adder(a, b, carry-in, sum, carry-out, agenda)
    {
    var ha1->ha2 = wire();
    var ha1->or = wire();
    var ha2->or = wire();

    half-adder(b,carry-in,ha1->ha2,ha1->or,agenda);
    half-adder(a,ha1->ha2,sum,ha2->or,agenda);
    OR(ha1->or,ha2->or,carry-out,agenda);
    }

function simulateHalfAdder()
    {
    var i;
    var a = wire();
    var b = wire();
    var sum = wire();
    var carry = wire();
    var simulator = agenda();
    var inputs = array(array(0,0), array(0,1), array(1,0), array(1,1));

    println("a\t\b\tsum\t\carry");

    half-adder(a, b, sum, carry, simulator);

    for (i = 0, i < length(inputs), i = i + 1)
	{
	a . set(inputs . i . 0);
	b . set(inputs . i . 1);
	simulator . run();
	println(inputs . i . 0, "\t", inputs . i . 1,
	    "\t",sum . get(), "\t", carry . get());
	}
    }

function simulateFullAdder()
    {
    var i;
    var a = wire();
    var b = wire();
    var carry-in = wire();
    var sum = wire();
    var carry-out = wire();
    var simulator = agenda();
    var inputs = array(
        array(0,0,0), array(0,0,1), array(0,1,0), array(0,1,1),
        array(1,0,0), array(1,0,1), array(1,1,0), array(1,1,1)
	);

    println("a\t\b\tc-in\tsum\t\c-out");

    full-adder(a, b, carry-in, sum, carry-out, simulator);

    for (i = 0, i < length(inputs), i = i + 1)
	{
	a . set(inputs . i . 0);
	b . set(inputs . i . 1);
	carry-in . set(inputs . i . 2);
	simulator . run();
	println(
	    inputs . i . 0, "\t", inputs . i . 1, "\t", inputs . i . 2, "\t",
	    sum . get(), "\t", carry-out . get()
	    );
	}
    }

simulateHalfAdder();
println();
simulateFullAdder();
