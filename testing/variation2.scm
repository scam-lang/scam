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
	    )
    this
    )

(define xo (x))
(define yo (y))

(println "(x()) . a() yields...");
(((new (x)) 'a))
(println "(y()) . a() yields...");
(((new (y)) 'a))
