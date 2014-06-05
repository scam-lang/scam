(define (g n)
    (define total 1)

    (while (> n 1)
        (set! total (* total n))
        (set! n (- n 1))
        )
    
    total
    )

(define x 10)

(print "g(" x ") is " (g x) "\n")
