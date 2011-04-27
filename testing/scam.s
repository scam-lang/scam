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
