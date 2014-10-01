(define (overloadMinus fileName)
    (define (iter x items count)
        (cond
            ((eof?) items)
            (else
                (inspect count)
                (iter (readToken) (cons x items) (+ count 1))
                )
            )
        )
    (inspect (iter (readToken) nil 0))
    )

(overloadMinus "words")

; run this with: cat words | scam bad.s
