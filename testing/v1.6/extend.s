(include "inherit.lib")
(define (square x) (* x x))
(define (wow)
    "wow, wow, wow!"
    )

(define (c x)
    (define parent nil)
    (define (wow) x)
    this
    )
(define (b y)
    (define parent (c (* y y)))
    (define (yow)
        (square (wow))
        )
    this
    )
(define (a z)
    (define parent (b (* z z)))
    (define (zow)
        (square (yow))
        )
    this
    )
(define obj (new (a 2)))

(println "wow() is " (wow))
(println "obj . wow() is " ((obj 'wow)))
(println "obj . yow() is " ((obj 'yow)))
(println "obj . zow() is " ((obj 'zow)))
