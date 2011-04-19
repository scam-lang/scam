(define (sqr x)
    (return (* x x))
    )

(define (a+b a b)
    (return (sqr (+ a b)))
    )

(define z  10)

(print "zeta of "  z  " and "  z  " is "  (a+b z z) "\n")
