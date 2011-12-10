(define (with obj $)
    (evalList $ obj)
    )


(define (f x) (define y (+ x 1)) this)

(define o (f 3))

(inspect (o 'x))
(inspect (o 'y))

(with (o)
    (set 'x 10)
    (set 'y 13)
    )

(inspect (o 'x))
(inspect (o 'y))
