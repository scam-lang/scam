(define (try # $error $a $b)
    (define result (catch (eval $a #)))
    (if (error? result)
        (begin
            (set $error result #)
            (set! result (eval $b #))
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
            (println "starting")
            ; comment the next uncommented line out and a 2 should result
            ; change hiy to hey in the next uncommented line and you get 100
            ; change hiy to hay in the next uncommented line and you get 33
            (throw 'hiy "ouch")
            (set! result (normalize 1 2))
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
