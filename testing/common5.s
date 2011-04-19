(include "inherit.s")

(define z = 
    (define z_shared (scope (define shared_count 0) this))

    (define (z)
        (define count 0)
        (extend z_shared)
        (set! 'count (+ count))
        (set! 'shared_count (+ shared_count 1))
        this
        )
    )

(define (x)
    (extend (z))
    )

(define (y)
    (extend (z))
    )

(define xish (x))
(define yish (y))
(define zish (z))

(println "x's common count is " (get 'shared_count xish)
(println "y's common count is " (get 'shared_count yish)
(println "z's common count is " (get 'shared_count zish)
(println "x's count is " (get 'count xish)
(println "y's count is " (get 'count yish)
(println "z's count is " (get 'count zish)
