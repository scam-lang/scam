(include "inherit.lib")

(define (+= # $v value)
    (set! (__id $v) (+ (eval $v #) value) #)
    )

(define common 0)

(define (z)
    (define parent nil)
    (define uncommon 0)
    (+= common 1)
    (+= uncommon 1)
    this
    )

(define (x)
    (define parent (z))
    this
    )

(inspect (get 'common (new (x))))
(inspect (get 'common (new (x))))
(inspect (get 'uncommon (new (x))))
(inspect (get 'uncommon (new (x))))
