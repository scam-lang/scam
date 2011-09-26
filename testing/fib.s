(println "starting fib.s")

(define (fib n)
    (if (< n 2)
        n
        (+ (fib (- n 1)) (fib (- n 2)))
        )
    )

(println "including fact.s")
(include "fact.s")

(define x 0)
(define result nil)
(define t (time))

(set! x 25)
(set! result (fib x))
(println "fib: fib(" x ") is " result)
(println (- (time) t) " seconds")

(println "fib.s included - SHOULD ONLY SEE THIS ONCE");
