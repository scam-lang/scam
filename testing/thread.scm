
(define (con A B @)
  (define (helper lst)
    (if (nil? lst)
      0
      (/ B (+ (car lst) (helper (cdr lst))))
    )
  )
  (+ A (helper @))
)

(define (fib n)
    (if (< n 2)
        n
        (+ (fib (- n 1)) (fib (- n 2)))
    )
)


(define (terrible seed)
    (define i (+ seed 1))
    (define j (+ seed 2))
    (define k (+ seed 3))
    (define l (+ seed 1))
    (while #t
      (displayAtomic (fib (int (con i j l (+ l 1) (+ l 2)))) "\n")
      (set! i (+ i 1))
      (set! j (+ j 1))
      (set! k (+ k 1))
      (set! l (+ l 10))
    )
)

(thread (terrible 0))
(thread (terrible 1))
(thread (terrible 2))
(thread (terrible 3))
(thread (terrible 4))
(thread (terrible 5))

