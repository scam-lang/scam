(include "wire.s")
(include "logic.s")
(include "agenda.s")

(define (half-adder a b sum carry simulator)
    (define or->and (wire))
    (define not->and (wire))

    (AND a b carry simulator)
    (OR a b or->and simulator)
    (NOT carry not->and simulator)
    (AND or->and not->and sum simulator)
    )

(define (full-adder a b carry-in sum carry-out agenda)
    (define ha1->ha2 (wire))
    (define ha1->or (wire))
    (define ha2->or (wire))

    (half-adder b carry-in ha1->ha2,ha1->or agenda)
    (half-adder a ha1->ha2 sum ha2->or agenda)
    (OR ha1->or ha2->or carry-out agenda)
    )

(define (simulateHalfAdder)
    (define i)
    (define a (wire))
    (define b (wire))
    (define sum (wire))
    (define carry (wire))
    (define simulator (agenda))
    (define inputs (array (array 0 0) (array 0 1) (array 1 0) (array 1 1)))

    (println "a\t\b\tsum\t\carry")

    (half-adder a b sum carry simulator)

    (for (assign i 0) (< i (length inputs)) (assign i (+ i 1))
        (a . set(inputs . i . 0);
        (b . set(inputs . i . 1);
	    ((. simulator run))
	    (println (getElement (. inputs i) 0) "\t", inputs . i . 1,
            "\t",sum . get(), "\t", carry . get());
	    )
    )

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
