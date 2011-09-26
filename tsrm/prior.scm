(include "reflection.lib")

(redefine (f) ((prior)) (println "starting first") ((prior)) (println "first"))
(redefine (f) (println "starting second") ((prior)) (println "second"))
(redefine (f) (println "starting third") ((prior)) (println "third"))
(redefine (f) (println "starting fourth") ((prior)) (println "fourth"))

(f)

(define (h) (println "starting first") (println "first"))
(scope
    (redefine (h) (println "starting second") ((prior)) (println "second"))
    (redefine (h) (println "starting third") ((prior)) (println "third"))
    (scope
        (redefine (h) (println "starting fourth") ((prior)) (println "fourth"))
        (h)
        )
    )

(define plus-count 0)
(redefine (+ a b)
    (assign plus-count ((prior) plus-count 1))
    ((prior) a b)
    )

(inspect plus-count)
(inspect (+ 3 4))
(inspect plus-count)

    (define x 0)
    (scope
        (define x 1)
        (define x 2)
        (println "x is " x)
        (define y (priorDefinition 'x x))
        (println "x was " y)
        (assign y (priorDefinition 'x y))
        (println "before that, x was " y)
        )
