(define (delay # $x) (lambda () (eval $x #)))
(define (force t) (t))

(define (sdisplay s n)
    (cond
        ((= n 0) (println "..."))
        (else (print (stream-car s) " ")  (sdisplay (force (cdr s)) (- n 1)))
        )
    )

(define (f x)
    (cons x (delay (f (+ x x))))
    )

(sdisplay (f 1) 10)

(define (sdisplay s n)
    (cond
        ((= n 0) (println "..."))
        (else (print (stream-car s) " ")  (sdisplay (stream-cdr s) (- n 1)))
        )
    )

(define (f x)
    (cons-stream x (f (+ x x)))
    )

(sdisplay (f 1) 10)

(define (psum s)
    (cons-stream
       (stream-car s)
       (psum (cons-stream (+ (stream-car s) (stream-car (stream-cdr s))) (stream-cdr (stream-cdr s))))
       )
    )

(define (add-streams s t)
    (cons-stream
        (+ (stream-car s) (stream-car t))
        (add-streams (stream-cdr s) (stream-cdr t))
        )
    )

(define ones (cons-stream 1 ones))
(define integers (cons-stream 1 (add-streams ones integers)))
(sdisplay ones 10)
(sdisplay integers 10)
(sdisplay (psum integers) 10)

(define (psum s)
    (cons-stream
        (stream-car s)
        (add-streams (stream-cdr s) (psum s))
        )
    )
(sdisplay (psum integers) 1000)

(define (psum s)
    (define ssum (cons-stream (stream-car s) (add-streams (stream-cdr s) ssum)))
    ssum
    )

(sdisplay (psum integers) 10)

