
(define (continuedFraction a b limit)
  (if (< limit 1)
    limit
    (/ a (+ b (continuedFraction a b (- limit 1))))
  )
)

(define (threadHelper)
  (define i 10)
  (define A (/ 1.0 (+ (gettid) 0.1)))
  (while #t
   (begin
        (displayAtomic (gettid) " : " (fmt "%0.15f" (continuedFraction A 2.0 i)) "\n")
        (if (> i 100)
          (set! i 10)
          (set! i (+ i 1))
        )
    )
  )
)


(thread (threadHelper)) ; 1
(thread (threadHelper)) ; 2
;(thread (threadHelper)) ; 3
;(thread (threadHelper)) ; 4
;(thread (threadHelper)) ; 5
;(thread (threadHelper)) ; 6
;(thread (threadHelper)) ; 7
;(thread (threadHelper)) ; 8
;(thread (threadHelper)) ; 9
;(thread (threadHelper)) ; 10
;(thread (threadHelper)) ; 11
;(thread (threadHelper)) ; 12
;(thread (threadHelper)) ; 13
(threadHelper)
