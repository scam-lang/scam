; Java style inheritance in three functions (really two, super is not needed)
; linear time

(include "reflection.s")

(define (resetClosures static obj)
    (define (update current rest)
        (if (closure? current) (set! 'context static current))
        (if (neq? rest nil) (update (car rest) (cdr rest)))
        )
    (define values (locals obj))
    ;(println "locals are " values)
    (if (!= values nil) (update (car values) (cdr values)))
    obj
    )

(define (inherit child parents reification static)
    ;(println "inherit " (list child parents reification static) "...")
    (if (null? parents)
        (set! 'context static child)
        (set! 'context
              (inherit (resetClosures reification (car parents))
                       (cdr parents) reification static)
              child
              )
        )
    child
    )

(define (new child)
    ;(println "new...")
    (define (chain x) (if (null? x) nil (cons x (chain (get 'parent x)))))
    (inherit child (chain (get 'parent child)) child (get 'context child))
    )

(define (mixin object @)
    (inherit object @ object (get 'context object))
    )

(define (super child)
    (get 'context child)
    )

(define (extend # parent)
    (define top
        (if (local? 'inherit-top parent) (get 'inherit-top parent) parent))
    
    ;(println "in extend...")
    (addSymbol 'inherit-top top #)
    
    (set! 'context (get 'context #) top)
    (set! 'context parent #)
    #
    )
