(define (up) (up))
(up)
;$
;(println "hello")
(define z 22)
(define (square x) (+ x x))
(define (fib n)
    (if (< n 2) n (+ (fib (- n 1)) (fib (- n 2))))
    )
(define (climb x)
   (println "x is " x)
   (climb (+ x 1))
   )

(define i 0)
(while (< i z)
    (println "i is " i)
    (set! i (+ i 1))
    )


(climb 0)
;$
(println "fib(" z ") is " (fib z))
