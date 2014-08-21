
(include "../thread.lib")

(debugMutex #t)

(define (run)
   (lock)
   (println "Running from thread" (gettid))
   (unlock)
)

(thread (run))
(thread (run))
(thread (run))
(thread (run))
