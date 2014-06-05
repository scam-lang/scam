(include "reflection.lib")

(redefine (while # $test $body $)
    (define oldwhile (prior))
    (define result (eval $test #))
    (if result
        (begin
            (eval $body #)
            (oldwhile (eval $test #)
                (eval $body #)
                )
            )
        (if (valid? $)
            (eval (car $) #)
            )
        )
    )

(define i)

(define i 0)
(while (< i 5)
    (begin
        (println "i is " i)
        (++ i)
        )
    )

(while (< i 5)
    (begin
        (println "i is " i)
        (++ i)
        )
    (println "i was too large!")
    )

