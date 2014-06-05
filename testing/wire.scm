(define (wire)
    (define oldSet set)
    (define value 0)  ;initial (nonsensical) value
    (define downstream nil)
    (define (register object)
        (oldSet 'downstream (cons object downstream))
        ((object 'recalculate))
        )
    (define (inform object)
        ((object 'recalculate))
        )
    (define (set newValue)
        (define d)
        (if (!= newValue value)
            (begin
                (oldSet 'value newValue)
                (map inform downstream)
                )
            )
        )
    (define (get) value)

    this
    )

