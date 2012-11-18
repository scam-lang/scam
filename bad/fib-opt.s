
(while #t
(define (fib n)
    (if (< n 2)
        n
        (+ (fib (- n 1)) (fib (- n 2)))
        )
    )

(define x 20)

(define result (fib x))
(inspect result)
)

