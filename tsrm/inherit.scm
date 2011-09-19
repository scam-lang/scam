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

(define obj (child))

(inspect ((dot obj b)))
(inspect ((dot obj a)))

