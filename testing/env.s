(include "scam.s")

(define spot ScamEnv)

(while (!= spot nil)
    (define s (car spot))
    (println (prefix s (stringUntil s "=")))
    (set! 'spot (cdr spot))
    )
