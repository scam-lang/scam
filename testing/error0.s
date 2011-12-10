(println "starting error0.s")
(include "error0.s")

(define (fib n)
    (if (< n 2)
        n
        (+ (fib (- n 1)) (fib (- n 2)))
        )
	)
(define x 0)
(define result)
(define t (time))

(define x 20)

(define result (fib x))
(println "fib(" x ") is " result)
(println (- (time) t) " seconds")
