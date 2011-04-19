(include "inherit.s")

(define (helper) (println "oops!"))

(define common 10)

(define (z)
    (define parent nil)
    (define (getCommon) (helper) common)
    (set! 'common (+ common 1))
    this
    )

(define (x)
    (define parent (z))
    (define common 1)
    (define (helper) (println "help!"))
    this
    )

(define xish (new (x)))

(inspect ((get 'getCommon (new (x)))))
(inspect ((get 'getCommon (new (x)))))
(inspect ((get 'getCommon (new (z)))))
(inspect ((get 'getCommon (new (z)))))
