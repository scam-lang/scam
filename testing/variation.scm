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
    (set* parent 'b
        (lambda ()
            (println "y replacement :b() -> au revoir")
            )
        )
    this
    )

(println "(x()) . a() yields...")
(((new (x)) 'a))
(println "(y()) . a() yields...")
(((new (y)) 'a))
