(println "starting fact.s")
(println "including fib.s")

(include "fib.s")

(define (fact n)
    (if (< n 2)
        1
        (* n (fact (- n 1)))
        )
    )

(println "fact: fib(5) is " (fib 5))
(println "fact: fact(5) is " (fact 5))

(println "fact.s included - SHOULD ONLY SEE THIS ONCE");
