
(define (f x)
    (define (g)
      (return (return 5))
      (* x 2)
    )
    (g)
 )

(display (f 5))
