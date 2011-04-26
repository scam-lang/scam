(println "starting fact.s")

(include "fib.s")

(define (fact n)
    (if (< n 2) 1 (* n (fact (- n 1))))
    )

(inspect this)

(println "fact: fib(5) is " (fib 5))
(println "fact: fact(5) is " (fact 5))
