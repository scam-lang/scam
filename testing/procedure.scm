(pp define)

(define old-define define)

(define (define # $)
    (
    (old-define $)
(define (define-function name params body env)
    (define donor (lambda () 1))
    (set! name name donor)
    (set! parameters params donor)
    (set! code body donor)
    (addSymbol name donor env)
    )

(define (loadAndgo # $name params values $body)
    (define f (define-function $name params $body #))
    (apply f values)
    )

(println "first load and go...");
(loadAndgo countdown '(a) '(4)
    (begin
        (inspect a)
        (if (> a 0) (countdown (- a 1)))
        )
    )

(println "second load and go...")
(loadAndgo countdown '(b) '(3)
    (begin
        (inspect b)
        (if (> b 0) (countdown (- b 1)))
        )
    )

(println "calling define-function directly...")
((define-function 'countdown '(a)
    '(begin
        (inspect a)
        (if (> a 0) (countdown (- a 1)))
        )
    this) 4)
