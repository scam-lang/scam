(include "inherit.lib")
(define (x)
    (define parent nil)
    (define (a)
        (println "x:a() -> hello")
        (b)
        )
    (define (b)
        (println "x:b() -> goodbye")
        )

    this
    )

(define (y)
    (define parent (x))
    (define (b)
        (println "y:b() -> au revoir")
        (print "run the old version of b, via parent: ")
        ((parent 'b))
        )
    this
    )

(define x-obj (new (x)))
(define y-obj (new (y)))

(println "x-obj . a() yields...")
((x-obj 'a))
(println "y-obj . a() yields...")
((y-obj 'a))
