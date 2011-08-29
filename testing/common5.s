(include "inherit.lib")

(define z
    (scope
        (define z_shared (scope (define shared-count 0) this))

        (define (z)
            (define count 0)
            (extend z_shared)
            this
            )
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

(set! 'count 1 yish)
(set! 'count 2 zish)

(set! 'shared-count 11 yish)
(set! 'shared-count 22 xish)

(println "x's count is " (get 'count xish))
(println "y's count is " (get 'count yish))
(println "z's count is " (get 'count zish))
(println "x's common count is " (get 'shared-count xish))
(println "y's common count is " (get 'shared-count yish))
(println "z's common count is " (get 'shared-count zish))
