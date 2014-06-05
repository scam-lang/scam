(define (f @)
    (if (== @ nil)
        0
        (+ (getElement @ 0) (apply f (cdr @)))
        )
    )

(define (g # $)
    (define total 0)
    (define size (length $))

    (while (> size 0)
        ;(inspect (- size 1))
        ;(inspect $)
        ;(inspect (getElement $ 0))
        ;(inspect (getElement $ (- size 1)))
        (set 'total (+ total (eval (getElement $ (- size 1)) #)))
        (set 'size (- size 1))
        )

    total
    )
(print "sum (via f): " (f 1 2 3 4) "\n")
(print "sum (via g): " (g 1 2 3 4) "\n")

(define (a)
    (define (f) (print "hello\n"))
    (define (g x) (println x))
    (lambda (@) (apply (get (car @) __context) (cdr @)))
    )

(define aa (a))
(aa'f)
(aa'g 'world)
