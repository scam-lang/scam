(define (mutex1)
	(define (p) (acquire))
	(define (v) (release))
	this
)

(define (run5)
  (allocateSharedMemory)
  (define m (mutex1))
  (define (f) (sleep 1) ((m'p)) (println "f") ((m'v)))
  (define (g) (sleep 1) ((m'p)) (println "g") ((m'v)))
  (define (h) (sleep 1) ((m'p)) (println "h") ((m'v)))
  (debugSemaphore #t)
  (pexecute (f) (g) (h))
  (freeSharedMemory)
)
(run5)
