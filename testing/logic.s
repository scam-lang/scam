(define (AND in1 in2 out agenda)
    (define delay 4)  ;milliseconds
    (define (recalculate)
        (if (and (== ((. in1 get)) 1) (== ((. in2 get)) 1))
            ((. agenda schedule) ((. out set) 1) delay)
            ((. agenda schedule) ((. out set) 0) delay)
            )
        )

    ((. in1 register) this)
    ((. in2 register) this)
    (recalculate)
    this
    )


(define (OR in1 in2 out agenda)
    (define delay 3) ;milliseconds
    (define (recalculate)
        if (or (== ((. in1 get)) 1) (== ((. in2 get)) 1))
            ((. agenda schedule) ((. out set) 1) delay)
            ((. agenda schedule) ((. out set) 0) delay)
        )

    ((. in1 register) this)
    ((. in2 register) this)
    this
    )


(define (NOT in out agenda)
    (define delay 1)  ;milliseconds
    (define (recalculate)
        (if (== ((. in get)) 0)
            ((. agenda schedule) ((. out set) 1) delay)
            ((. agenda schedule) ((. out set) 0) delay)
            )
        )

    ((. in register) this)
    this
    )
