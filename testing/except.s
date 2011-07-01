(define (try # $a $b)
    (define result (catch (eval $a #)))
    (if (== (type result) 'error)
        (eval $b #)
        result
        )
    )

(define (fact n)
    (if (== n 0)
        (try
            zzz ;change this line to 1
            (begin
                (println "[ERROR]")
                1
                )
            )
        (* n (fact (- n 1)))
        )
    )

(print "fact(5) is "  (fact 5) "\n")
