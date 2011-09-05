(include "priorityQueue.s")

(define (agenda)
    (define time 0)
    (define actions (priorityQueue))
    (define (schedule # $action delay)          ;action is delayed!
        ((. actions enqueue) $action  (+ time delay))
        )
    (define (run)
        (while (not((. actions empty)))
            (assign time ((. actions peekRank)))
            (force ((. actions dequeue)))       ;action was delayed!
            )
        (assign time 0)
        )

    this
    )
