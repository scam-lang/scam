(define (head x) (car x))
(define (tail x) (cdr x))
(define (join x,y) (cons x y))

(define (for # init $test $increment $body)
    (while (eval $test #)
        (eval $body #)
        (eval $increment #)
        )
    )

(define (for-each # $indexVar items $body)
    (define result false)
    (while (!= items nil)
        (set! $indexVar (car items) #)
        (set! 'result (eval $body #))
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
