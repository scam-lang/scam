

(define (f)
    (define (g)
        (return (return (return 5)))
        (display "LOL\n")
    )
    (g)
)
(display "Before\n")
(display (f))
(display "\nAfter\n")
