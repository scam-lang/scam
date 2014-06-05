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
        (set 'total (+ total (eval (getElement $ (- size 1)) #)))
        (set 'size (- size 1))
        )

    total
    )
(print "sum (via f): " (f 1 2 3 4) "(should be 10)\n")
(print "sum (via g): " (g 1 2 3 4) "(should be 10)\n")

(define (a)
    (define (f) (print "hello\n"))
    (define (g x) (println x))
    (lambda (@) (apply (get (car @) __context) (cdr @)))
    )

(define aa (a))
(aa'f)
(aa'g 'world)
