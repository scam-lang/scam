(define (foreach # $v items $body)
    (while (valid? items)
        (set $v (car items) #)
        (eval $body #)
        (set 'items (cdr items))
        )
    )

(define i)
(define a (array 1 2 3 4))

(foreach i a
    (inspect i)
    )
