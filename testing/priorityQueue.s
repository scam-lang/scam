(define (priorityQueue)
    (define items)
    (define (bundle data rank next) this)
    (define (dequeue)
        (define item)
        
        (assign item (. items next data))
        (assign (. items next) (. items next next))
        item
        )
    (define (enqueue item rank)
        (define prefix items)
        (define suffix (. items next))
        (define package (bundle item rank nil))

        (while (and (valid? suffix) (>= rank (. suffix rank)))
            (assign prefix suffix)
            (assign suffix (. suffix next))
            )

        (assign (. prefix next) package)
        (assign (. package next) suffix)
        )
    (define (peekItem) (. items next data))
    (define (peekRank) (. items next rank))
    (define (empty)    (null? (. items next)))

    (assign items (bundle nil nil nil))  ;dummy head node
    this
    )
