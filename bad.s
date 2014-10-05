(define (overloadMinus fileName)
    (define (iter x items count)
        (cond
            ((eof?) items)
            (else
                (inspect count)
                ; Jeff : After every GC print the items
                ;(if (gced) 
                ;    (inspect items)
                ;)
                (iter (readToken) (cons x items) (+ count 1))
                )
            )
        )
    (inspect (iter (readToken) nil 0))
    )

(overloadMinus "words")

; run this with: cat words | scam bad.s
; Jeff: or with the debugger: scam bad.s < words 
