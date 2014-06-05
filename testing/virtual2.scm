(include "inherit.lib")

(define (a)
    (define parent nil)
    (define x 0)
    (define (display) (println "a: x is " x))
    this
    )
(define (b)
    (define parent (a))
    (define x 1)
    (define (display) (println "b: x is " x))
    this
    )

(define aish (new (a)))
(define bish (new (b)))

((aish 'display))
((bish 'display))
((bish 'parent 'display))
