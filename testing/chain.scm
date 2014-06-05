(println "testing clientship in objects")
(define (a x y)
    this
    )

(define (b m n)
    (define o (a 'one 'two))
    this
    )

(define obj (b 'red 'green))

(inspect (obj'o'x))
(println "output should have been 'one'")
(inspect (obj'o'y))
(println "output should have been 'two'")
(inspect (set! y 'three (obj'o)))
(inspect (obj'o'y))
(println "output should have been 'three")
