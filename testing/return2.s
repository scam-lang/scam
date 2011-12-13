(define size 17)

(define (heapify root)
    (return 'ok))
    
(define (build-heap)
    (for (define i (- size 1)) (>= i 0) (-- i)
        (println "heapifying index " i)
        (heapify i)
        (println "index " i " has been heapified")
        )
    )

(build-heap)
