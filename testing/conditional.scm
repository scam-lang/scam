(for (define i 0) (< i 4) (++ i)
    (cond
        ((= i 0) (println 'zero))
        ((= i 1) (println 'one))
        ((= i 2) (println 'two))
        (else (println "more than two"))
        )
    )
(inspect else)
