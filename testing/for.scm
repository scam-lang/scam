(define i 0)

(define (for # init $test $increment $)
    (while (eval $test #)
        (evalList $ #)
        (eval $increment #)
        )
    )

(define (for2 # init $test $increment $)
    (define (iter env)
        (if (eval $test env)
            (begin
                (evalList $ env)
                (eval $increment env)
                (iter env)
                )
            )
        )
    (iter #)
	)

(for (set! i 1) (< i 10) (set! i (+ i 2))
    (print "the value of i is ")
    (print i)
    (print "\n")
    )
(for2 (set! i 1) (< i 10) (set! i (+ i 2))
    (print "the value of i is ")
    (print i)
    (print "\n")
    )
