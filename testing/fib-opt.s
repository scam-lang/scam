(include "compile.lib")
(include "pretty.lib")

(define (fib n)
    (if (< n 2)
        n
        (+ (fib (- n 1)) (fib (- n 2)))
        )
    )

(define x 25)
(define result)

(define s (time))
(define result (fib x))
(define f (time))
(println "(fib " x ") is " result ", " (- f s) " seconds.")

(compile fib this)

(define s (time))
(define result (fib x))
(define f (time))
(println "(fib " x ") is " result ", " (- f s) " seconds.")
