(define i 0)
(while (< i 4)
    (println (int (* (sin i) 1000)))
    (+= i .1)
    )