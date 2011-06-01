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
    (define result #f)
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

(define scam.s-old-plus +)
(define (+ @)
    (define (iter result items)
        (if (null? items)
            result
            (iter (scam.s-old-plus result (car items)) (cdr items))
            )
        )
    (iter 0 @)
    )
(define scam.s-old-times *)
(define (* @)
    (define (iter result items)
        (if (null? items)
            result
            (iter (scam.s-old-times result (car items)) (cdr items))
            )
        )
    (iter 1 @)
    )
(define scam.s-old-minus -)
(define (- @)
    (define (iter result items)
        (if (null? items)
            result
            (iter (scam.s-old-minus result (car items)) (cdr items))
            )
        )
    (cond 
        ((null? @) 0)
        ((null? (cdr @)) (iter 0 @))
        (else (iter (car @) (cdr @)))
        )
    )
(define scam.s-old-divides /)
(define (/ @)
    (define (iter result items)
        (if (null? items)
            result
            (iter (scam.s-old-divides result (car items)) (cdr items))
            )
        )
    (cond 
        ((null? @) 1)
        ((null? (cdr @)) (iter 1 @))
        (else (iter (car @) (cdr @)))
        )
    )
(define (dec x) (- x 1))
(define (inc x) (+ x 1))

(define (let # $inits $)
    (define v nil)
    (define e (scope this))
    (set! 'context # e)
    (for-each v $inits
        (addSymbol (car v) (eval (car (cdr v)) #) e)
        (inspect e)
        )
    (evalList $ e)
    )

(define (let* # $inits $)
    (define v nil)
    (define e (scope this))
    (set! 'context # e)
    (for-each v $inits
        (addSymbol (car v) (eval (car (cdr v)) e) e)
        )
    (evalList $ e)
    )

(define (negative? n) (< n 0))
(define (positive? n) (> n 0))

(define (newline) (println))
(define (display x) (print x))

