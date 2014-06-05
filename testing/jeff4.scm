
(define (f x)
    (define (g)
      (return (return 5))
      (* x 2)
    )
    (g)
    (+ x 1)
 )

(display (f 5))
