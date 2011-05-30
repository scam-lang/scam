(define (head x) (car x))
(define (tail x) (cdr x))
(define (join x,y) (cons x y))

(define (for # init $test $increment $)
    (while (eval $test #)
        (evalList $ #)
        (eval $increment #)
        )
    )

(define (for-each # $indexVar items $)
    (define result false)
    (while (!= items nil)
        (set! $indexVar (car items) #)
        (set! 'result (evalList $ #))
        (set! 'items (cdr items))
        )
    result
    )

(define (+= # $v value) (set! $v (+ (eval $v #) value) #))
(define (-= # $v value) (set! $v (- (eval $v #) value) #))
(define (*= # $v value) (set! $v (* (eval $v #) value) #))
(define (/= # $v value) (set! $v (/ (eval $v #) value) #))

; object-related functions

(define old-type type)

(define (type x)
    (if (== (old-type x) 'object) (get 'label x) x)
    )

(define (. obj $field) (get $field obj))

(define old-plus +)
(define (+ @)
    (define (iter result items)
        (if (null? items)
            result
            (iter (old-plus result (car items)) (cdr items))
            )
        )
    (iter 0 @)
    )
(define old-times *)
(define (* @)
    (define (iter result items)
        (if (null? items)
            result
            (iter (old-times result (car items)) (cdr items))
            )
        )
    (iter 1 @)
    )
(define old-minus -)
(define (- @)
    (define (iter result items)
        (if (null? items)
            result
            (iter (old-minus result (car items)) (cdr items))
            )
        )
    (cond 
        ((null? @) 0)
        ((null? (cdr @)) (iter 0 @))
        (else (iter (car @) (cdr @)))
        )
    )
(define old-divides /)
(define (/ @)
    (define (iter result items)
        (if (null? items)
            result
            (iter (old-divides result (car items)) (cdr items))
            )
        )
    (cond 
        ((null? @) 1)
        ((null? (cdr @)) (iter 1 @))
        (else (iter (car @) (cdr @)))
        )
    )
