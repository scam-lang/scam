(define (f value)
    (inspect value)
    (return value)
    'ok
    )
    
(define (main)
    (println "calling f 16 times")
    (define i 16)
    (for 'ok (>= i 0) (-- i)
        (println "calling f with " i)
        ;(+ (f i) 0)                        ;this works
        (f i)                               ;this doesn't
        (println "f was called with " i)
        )
    )
(define (main2)
    (println "calling f 16 times")
    (define i 16)
    (while (> i 0)
        (println "calling f with " i)
        ;(+ (f i) 0)                        ;this works
        (f i)                               ;this doesn't
        (println "f was called with " i)
        (-- i)
        )
    )


(main)
(main2)
