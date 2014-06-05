(include "inherit.lib");

(println "test of a static ancestor class versus a dynamic ancestor class...")

(define z-common (scope (define count 0) this)) ; a static ancestor class

(define (z)
    (define parent nil)
    (define common z-common)
    (define uncommon (scope (define count 0) this)) ; a dynamic ancestor class
    this
    )

(define (x)
    (define parent (z))
    (set! count (+ (parent'common'count) 1) (parent'common))
    (set! count (+ (parent'uncommon'count) 1) (parent'uncommon))
    this
    )

(define (y)
    (define parent (z))
    (set! count (+ (parent'common'count) 1) (parent'common))
    (set! count (+ (parent'uncommon'count) 1) (parent'uncommon))
    this
    )

(define xish (new (x)))
(define yish (new (y)))
(define zish (new (z)))

(println "x's common count is " (xish'common'count))
(println "y's common count is " ( yish'common'count))
(println "    the above counts should both be 2")
(println "x's uncommon count is " (xish'uncommon'count))
(println "y's uncommon count is " (yish'uncommon'count))
(println "    the above counts should both be 1")
