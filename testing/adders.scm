(include "wire.scm")
(include "logic.scm")
(include "agenda.scm")

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

    (half-adder b carry-in ha1->ha2 ha1->or agenda)
    (half-adder a ha1->ha2 sum ha2->or agenda)
    (OR ha1->or ha2->or carry-out agenda)
    )

(define (simulateAND)
    (define i)
    (define a (wire))
    (define b (wire))
    (define c (wire))
    (define simulator (agenda))
    (define inputs (array (array 0 0) (array 0 1) (array 1 0) (array 1 1)))

    (println "a\t\b\tc")

    (AND a b c simulator)

    (for (set 'i 0) (< i (length inputs)) (set 'i (+ i 1))
        ((a 'set) (getElement inputs i 0))
        ((b 'set) (getElement inputs i 1))
	    ((simulator 'run))
	    (println
            (getElement inputs i 0) "\t" 
            (getElement inputs i 1) "\t" 
            ((c 'get))
            )
	    )
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

    (for (set 'i 0) (< i (length inputs)) (set 'i (+ i 1))
        ((a 'set) (getElement inputs i 0))
        ((b 'set) (getElement inputs i 1))
	    ((simulator 'run))
	    (println
            (getElement inputs i 0) "\t" 
            (getElement inputs i 1) "\t" 
            ((sum 'get)) "\t"
            ((carry 'get))
            )
	    )
    )

(define (simulateFullAdder)
    (define i)
    (define a (wire))
    (define b (wire))
    (define carry-in (wire))
    (define sum (wire))
    (define carry-out (wire))
    (define simulator (agenda))
    (define inputs (array 
        (array 0 0 0)  (array 0 0 1)  (array 0 1 0)  (array 0 1 1) 
        (array 1 0 0)  (array 1 0 1)  (array 1 1 0)  (array 1 1 1)
        ))

    (println "a\t\b\tc-in\tsum\t\c-out")

    (full-adder a  b  carry-in  sum  carry-out  simulator)

    (for (set 'i 0) (< i (length inputs)) (set 'i (+ i 1))
        ((a 'set) (getElement inputs i 0))
        ((b 'set) (getElement inputs i 1))
        ((carry-in 'set) (getElement inputs i 2))
        ((simulator 'run))
        (println 
            (getElement inputs i 0)  "\t" 
            (getElement inputs i 1)  "\t" 
            (getElement inputs i 2)  "\t" 
            ((sum 'get))  "\t"  ((carry-out 'get))
            )
        )
    )

(simulateAND)
(println)
(simulateHalfAdder)
(println)
(simulateFullAdder)
