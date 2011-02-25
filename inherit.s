; Java style inheritance in three functions (really two, super is not needed)
; linear time

(include "object.s")

; set up classes, GP is grandparent class, P is parent class, C is child class

(define GP
    (scope
        (define zzz 3)
        (define (GP)
            (define (a)
                (println "grandparent a")
                )
            (define (b)
                (println "grandparent b, calling a")
                (a)
                )
            (define (c)
                (println "grandparent c, calling b")
                (b)
                )
            this
            )
        ;(inspect this)
        ;(inspect (get 'context GP))
        ;GP
        )
    )

(define (P)
    (define parent (GP))
    (define (a)
        (println "parent a")
        )
    (define (b)
        (println "parent b, calling a")
        (a)
        )
    this
    )

(define (C)
    (define parent (P))
    (define (a)
        (println "child a")
        )
    this
    )

; now test

(define c (new C))

(println "child has function a, parent has functions a and b, ")
(println "grandparent has functions a, b, and c")
(println)
(println "calling child's a...")
(println)
((get 'a c))

(println)
(println "calling child's b...")
(println)
((get 'b c))
(println)

(println "calling child's c...")
(println)
((get 'c c))
(println)

(println "calling parent's a...")
(println)
((get 'a (super c)))
(println)

(println "calling parent's b...")
(println)
((get 'b (super c)))
(println)

(println "calling parent's c...")
(println)
((get 'c (super c)))
(println)

(println "calling grandparent's a...")
(println)
((get 'a (super (super c))))
(println)

(println "calling grandparent's b...")
(println)
((get 'b (super (super c))))
(println)

(println "calling grandparent's c...")
(println)
((get 'c (super (super c))))
(println)

(define child c)
(println "make sure child object's static scope is enforced...")
(print   "    this should succeed:   ")
(inspect (get 'zzz (new GP)))
(println "    this should fail:      (get 'zzz child)")
(get 'zzz child)
