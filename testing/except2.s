(include "scam.s")

(define (try # $error $a $b)
    (define result (catch (eval $a #)))
    (if (== (type result) 'error)
        (begin
            (set! $error result #)
            (set! 'result (eval $b #))
            )
        )
    result
    )

(define (normalize a b)
    (* (+ a b) (+ a b))
    )

(define (g)
    (define result)
    (define error)

    (try error 
        (begin
            (throw 'hiy "ouch") // comment this line out and a 2 should result
            (set! 'result (normalize 1 2))
            )
        (if (== (get 'code error) 'nonFunction)
            (begin
                (println "not a function!")
                (set! 'result 2)
                )
        (if (== (get 'code error) 'hey)
            (set! 'result 100)
        (if (== (get 'code error) 'hay)
            (set! 'result 33)
            (throw error)
            )))
        )
    result
    )

(define + 3)
(println (g))
