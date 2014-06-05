(define (fwhile # $test $)
    (while (eval $test #)
        (evalList $ #)
        )
	)
(define (f x)
    (fwhile (< i 10)
        (set 'j 0)
        (fwhile (< j 10)
            (print i " * " j " is " (* i j) "\n")
            (if (= (* i j) x)
                (return 0)
                )
            (++ j)
            )
        (++ i)
        )
    )

(define i 0)
(define j 0)
(define stop 9)

(print "run through i and j, stopping when i * j is " stop "\n")

(f stop)

(print "done\n")