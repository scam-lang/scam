(define words nil)

(define (overloadMinus fileName)
    (define p (open fileName 'read))
    (define (iter x items count)
        (cond
            ((eof?) items)
            (else
                (inspect count)
                (iter (readToken) (cons x items) (+ count 1))
                )
            )
        )
    (define oldPort (setPort p))
    (set! words (iter (readToken) nil 0))
    (close p)
    (setPort oldPort)
    (inspect words)
    (inspect (length words))
    )

(overloadMinus "words")
