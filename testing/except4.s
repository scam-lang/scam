(include "scam.s");

(define (f x)
    (define result nil)
    (println "beginning f(" x ").")
    (set! 'result (catch (g (* 2 x))))
    (if (== (type result) 'error)
        (if (== (get 'code result) 'undefinedVariable)
            (set! 'result x)
            (begin
                (println "rethrowing the error");
                (throw result)
                )
            )
        )
    (println "done with f(" x ").")
    result
    )
(define (g y)
    (define result nil)
    (println "beginning g(" y ").")
    //(throw 'undefinedVariable "take that!")
    (throw 'myError "take that!")
    (set! 'result (* mnenom y))
    (println "done with g(" y ").")
    result
    )
(println (f 4))
