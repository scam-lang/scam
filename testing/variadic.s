(define (show @)
    (apply print @)
    )

(define (fatal @)
    (apply print @)
    (exit 3)
    )

(show "goodbye, " "cruel" " world")
(print "\n");
(fatal "goodbye, " "cruel" " world")
