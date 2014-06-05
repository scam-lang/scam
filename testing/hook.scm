(define olddefine define)
(define (define # $)
    (println "intercepted! initializer is " (cdr $))
    (pass olddefine # $)
    )

(define (greeting)
    (define msg "hello, world!")
    (println msg)
    )

(println "about to greet...")
(greeting)
