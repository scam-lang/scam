(include "inherit.lib")

(define (x)
    (define parent nil)
    (define a 3)
    this
    )

(define (y)
    (define parent (x))
    (define b 5)
    this
    )

(define z (new (y)))
(println "z = y(); z . a is " (. z a))
(println "new(y()) . a is " (. (new (y)) a))
