(define (overloadMinus fileName)
    (define (iter x items count)
        (cond
            ((= count 10000000) items)
            (else
                (inspect count)
                (iter '(readToken) (cons (string 'x1234567890) items) (+ count 1))
                )
            )
        )
    (iter '(readToken) nil 0)
    )

(overloadMinus "words")
