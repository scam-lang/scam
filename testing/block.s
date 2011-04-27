(include "scam.s")

(define i 0)

(define (for2 # init $test  $increment  $)
    (while (eval $test #)
        (evalList $ #)
        (eval $increment #)
        )
    )

(for (set! 'i 1) (< i 10) (set! 'i (+ i 2))
        (print "the value of i is ")
        (print i)
        (print "\n")
    )

(for2 (set! 'i 10) (> i 0) (set! 'i (- i 1)) 
        (print "the value of i is ")
        (print i)
        (print "\n")
    )
