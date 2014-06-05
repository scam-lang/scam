(define test (allocate 2))
(define (fib n)
    (set! test (allocate 2))
    (if (< n 2)
        n
        (+ (fib (- n 1)) (fib (- n 2)))
        )
    )


(define x 0)
(define result nil)
(define t (time))

(set 'x 22)
(set 'result (fib x))
(display result)
(display "\n")
(display (- (time) t))
(display " seconds\n")

