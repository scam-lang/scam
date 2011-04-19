(define (cddddr x) (cdr (cdr (cdr (cdr x)))))

(define (class x) (get 'label x))

(define (closure? x)
    (and (and (pair? x) (eq? (car x) 'object)) (eq? (class x) 'closure))
    )

(define (object? x)
    (and (pair? x) (eq? (car x) 'object))
    )

(define (local? id env)
    (member? id (car (cdr env)))
    )

(define (defined? id env)
    (define result (catch (get id env)))
    (!= (type result) 'ERROR)
    )

(define (locals env)
    (cddddr (car (cdr (cdr env))))
    )

(define (. obj $field) (get $field obj))

(define (member? x items)
    (cond
        ((null? items)
            #f
            )
        ((eq? x (car items))
            #t
            )
        (else
            (member? x (cdr items))
            )
        )
    )

