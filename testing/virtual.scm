(include "inherit.lib")
(define x-shared 
    (scope
        (define count 0)
        (define (+count ) (++ count))
        this
        )
    )

(define (x)
    (define parent nil)
    (define shared x-shared)
    (define trials 5)
    (define (a)
        (print trials ": x's a")
        (if (> trials 0)
            (begin
                (println ", calling b")
                (-- trials)
                (b)
                )
            (println)
            )
        )
    (define (b)
        (print "   x's b, calling a\n")
        (a)
        )

    ((shared '+count))

    this
    )

(define (y)
    (define parent (x))
    (define (a)  ;override x's a
        (print "y's a, calling b\n")
        (b)
        )
    (define (b)  ;override x's b
        (print "y's b, calling x's version of a\n")
        ((parent 'a))
        )

    (set* parent 'trials (+ (parent 'trials) 2))
    this
    )

(define xish (new (x)))
(define yish (new (y)))

(println "number of x objects created: " (xish 'shared 'count))
(println "number of x objects created, via y: " (yish 'shared 'count))
((xish 'a))
(println "------------------------------")
((yish 'a))
