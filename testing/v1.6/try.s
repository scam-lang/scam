(define (try # $store $code $alternative)
    (define result (catch (eval $code #)))
    (if (error? result)
        (begin
            (set $store result #)
            (eval $alternative #)
            )
        )
    result
    )

(define error)
(define a 6)
(define b 0)

(try error
    (/ a b)
    (if (eq? (error 'code) 'undefinedVariable)
        (println "try block has an undefined variable")
        (println "try block has a divide error")
        )
    )
(try error
    (/ a c)
    (if (eq? (get* error 'code) 'undefinedVariable)
        (println "try block has an undefined variable")
        (println "try block has a divide error")
        )
    )
