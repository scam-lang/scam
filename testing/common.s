(include "inherit.lib");

(define z-common (scope (define count 0) this))

(define (z)
    (define parent nil)
    (define common z-common)
    (define uncommon (scope (define count 0) this))
    this
    )

(define (x)
    (define parent (z))
    (assign (. (. parent common) count) (+ (. (. parent common) count) 1))
    (assign (. (. parent uncommon) count) (+ (. (. parent uncommon) count) 1))
    this
    )

(define (y)
    (define parent (z))
    (assign (. (. parent common) count) (+ (. (. parent common) count) 1))
    (assign (. (. parent uncommon) count) (+ (. (. parent uncommon) count) 1))
    this
    )

(define xish (new (x)))
(define yish (new (y)))
(define zish (new (z)))

(println "x's common count is " (. (. xish common) count))
(println "y's common count is " (. (. yish common) count))
(println "x's uncommon count is " (. (. xish uncommon) count))
(println "y's uncommon count is " (. (. yish uncommon) count))
