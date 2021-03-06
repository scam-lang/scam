(include "inherit.lib")

(define z
    (scope
        (define z_shared
            (scope
                (define parent nil)
                (define shared-count 0)
                this
                )
            )

        (define (z)
            (define parent z_shared)
            (define count 0)
            (assign count (+ count 1))
            (assign (. parent shared-count) (+ (. parent shared-count) 1))
            this
            )
        )
    )

(define (x)
    (define parent (z))
    )

(define (y)
    (define parent (z))
    )

(define xish (new (x)))
(define yish (new (y)))
(define zish (new (z)))

(println "x's common count is " (get 'shared-count xish))
(println "y's common count is " (get 'shared-count yish))
(println "z's common count is " (get 'shared-count zish))
(println "x's count is " (get 'count xish))
(println "y's count is " (get 'count yish))
(println "z's count is " (get 'count zish))
