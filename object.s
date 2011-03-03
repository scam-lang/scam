; Java style inheritance in three functions (really two, super is not needed)
; linear time

(define (cddddr x) (cdr (cdr (cdr (cdr x)))))

(define (class x) (get 'label x))

(define (closure? x)
    (and (pair? x) (and (eq? (car x) 'object) (eq? (class x) 'closure))))

(define (object? x) (and (pair? x) (eq? (car x) 'object)))

(define (member? x items)
    (cond
        ((null? items) #f)
        ((eq? x (car items)) #t)
        (else (member? x (cdr items)))
        )
    )

(define (local? id env)
    (member? id (car (cdr env)))
    )

(define (locals env)
    (cddddr (car (cdr (cdr env))))
    )

(define (new original)
    (define (update static child)
        (define parent (get 'parent child))
        (if (null? parent)
            (set! 'context static child)
            (begin
                (set! 'context parent child)
                (resetClosures original parent)
                (update static parent))))
    (update (get 'context original) original)
    original
    )

(define (mixin original parent @)
    (define (update static child parent rest)
        (resetClosures original parent)
        (set! 'context parent child)
        (if (null? rest)
            (set! 'context static parent)
            (update static parent (car rest) (cdr rest)))
        )
    (update (get 'context original) original parent @)
    original
    )

(define (inherit object parents reification static)
    (cond
        ((null? parents)
            (set! 'context static object))
        (else
            (set! 'context 
                (inherit (resetClosures reification (car parents))
                       (cdr parents) reification static)
                object))
        )
    object
    )

(define (mixin object @)
    (inherit object @ object (get 'context object))
    )

(define (new object)
    (define (chain x) (if (null? x) nil (cons x (chain (get 'parent x)))))
    (inherit object (cdr (chain object)) object (get 'context object))
    )

(define (resetClosures static obj)
    (define (update current rest)
        (if (closure? current) (set! 'context static current))
        (if (neq? rest nil) (update (car rest) (cdr rest)))
        )
    (define values (locals obj))
    (if (neq? values nil) (update (car values) (cdr values)))
    obj
    )

(define (super child)
    (get 'context child)
    )
