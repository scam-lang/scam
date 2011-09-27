(define (a x y)
    this
    )

(define (b m n)
    (define o (a 'one 'two))
    this
    )

(define obj (b 'red 'green))

(println "output should be 'one'")
(inspect (. obj o x))
(println "output should be 'two'")
(inspect (. obj o y))
