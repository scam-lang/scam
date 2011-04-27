(define (f x)
    (define y (* x x))

    (gc)

    (inspect f)
    (inspect x)
    (inspect y)
    )

(f 3)
