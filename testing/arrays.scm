; allocate* allocates multi-dimensional arrays
(define (allocate* @)
    (cond
        ((null? @) nil)
        (else
            (define a (allocate (car @)))
            (for (define i 0) (< i (car @)) (++ i)
                (setElement a i (apply allocate* (cdr @)))
                )
            a
            )
        )
    )

(inspect (allocate 5))
(inspect (allocate* 5))
(inspect (allocate* 5 4))
(inspect (allocate* 5 4 3 2))
