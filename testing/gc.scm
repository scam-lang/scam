(define (f x)
    (define y (* x x))

    (inspect f)

    (gc)

    (inspect x)
    (inspect y)
    )

(f 3)
