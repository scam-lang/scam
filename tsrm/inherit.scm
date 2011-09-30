(include "inherit.lib")

(define (c) "happy")
(define (parent)
    (define (b) "slap")
    this
    )
(define (child)
    (define (a) "jacks")
    (define temp (parent))
    (setEnclosingScope temp (getEnclosingScope this))
    (setEnclosingScope this temp)
    this
    )


(define y (child))

(inspect ((dot y b)))
(inspect ((dot y a)))

(define (a)
    (define parent nil)
    this
    )
(define (b)
    (define parent (a))
    this
    )

(define x (a))
(define y (b))

(inspect (is? x 'a))
(inspect (is? y 'b))
(inspect (is? y 'a))
(inspect (is? x 'b))

