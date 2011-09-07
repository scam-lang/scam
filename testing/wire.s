(define (wire )
    (define value 0)  ;initial (nonsensical) value
    (define downstream nil)
    (define (register object)
        (assign downstream (cons object downstream))
        ((. object recalculate))
        )
    (define (inform object)
        ((. object recalculate))
        )
    (define (set newValue)
        (if (!= newValue value)
            (begin
                (assign value newValue)
                (map inform downstream)
                )
            )
        )
    (define (get) value)

    this
    )

