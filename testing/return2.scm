(define (heapify root)
    (println "g level: "  __level)
    (return 'ok)
    'ok
    )
    
(define (build-heap)
    (println "build-heap level: " __level)
    (define i 16)
    (for 'ok (>= i 0) (-- i)
        (println "heapifying index " i)
        (println "loop body level: " __level)
        (readRawChar)
        (heapify i)
        (println "index " i " has been heapified")
        )
    )

(build-heap)
