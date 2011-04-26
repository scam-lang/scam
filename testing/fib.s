(println "starting fib.s")

(include "fact.s")

(println "done including fact.s")

(define (fib n)
    (if (< n 2)
        n
        (+ (fib (- n 1)) (fib (- n 2)))
        )
    )
(define x 0)
(define result nil)
(define t (time))

(set! 'x 26)
(set! 'result (fib x))
(println "fib: fib(" x ") is " result)
(println (- (time) t) " seconds")
