(define x
    (scope
        (define a 2)
        (define b "hello")
        (println "the shared environment was made!")
        (lambda ()
            (define c 5)
            (println "an object was made!\n")
            this
            )
        )
    )

(define y (x))
(define z (x))

(inspect z)

(println "y:(a,b,c) is " (. y a) "," (. y b) ","  (. y c))
(println "z:(a,b,c) is " (. z a) "," (. z b) ","  (. z c))

(println "changing a and c for y")

(assign (. y a) 3)
(assign (. y c) 4)

(println "y:(a,b,c) is " (. y a) "," (. y b) "," (. y c))
(println "only a should have changed for z")
(println "z:(a,b,c) is " (. z a) "," (. z b) "," (. z c))
