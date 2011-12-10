(define (f a)
    this
    )

(define obj (f 2))

(set* obj 'a 4)

(define x 'a)

(println "obj . a is " (obj 'a))
(println "obj . a is " (get 'a obj))
(println "obj . a is " (get* obj x))
