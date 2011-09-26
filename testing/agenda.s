(include "priorityQueue.s")

(define (agenda)
    (define time 0)
    (define actions (priorityQueue))
    (define (schedule # $action delay)          ;action is delayed!
        (define result (thunk $action #))
        ;(print "enqueuing " result)
        ;(inspect result)
        ((. actions enqueue) result  (+ time delay))
        )
    (define (run)
        (while (not((. actions empty)))
            (assign time ((. actions peekRank)))
            (define result ((. actions dequeue)))
            ;(println "dequeuing " result)
            ;(inspect result)
            (force result)
            )
        (assign time 0)
        )

    this
    )
