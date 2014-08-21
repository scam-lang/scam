(include "../thread.lib")

(define (inner)
  (unlock)
  (displayAtomic "After unlock\n")
)

(define (outer)
   (lock)
  (displayAtomic "After lock\n")
   (thread (inner))
  (displayAtomic "After inner\n")
)

(thread (outer))
