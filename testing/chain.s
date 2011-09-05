(define (a x y)
    this
    )

(define (b m n)
    (define o (a 'one 'two))
    this
    )

(define obj (b 'red 'green))

(inspect (. obj o x))

