(include "stream.scm")

(define (ln2-summands n)
    ;(println "ln2-summands stack depth: " (stack-depth))
    (cons-stream (/ 1.0 n)
        (stream-map - (ln2-summands (+ n 1)))
        )
    )
(define (ln2-summands n f)
    ;(println "ln2-summands stack depth: " (stack-depth))
    (cons-stream (/ 1.0 n f)
        (ln2-summands (+ n 1) (* f -1))
        )
    )

(define ln2-stream (partial-sum (ln2-summands 1 1)))
;(define ln2-stream (partial-sum (ln2-summands 1)))

(define ln2 ln2-stream)

(define (run7)
    (println "(sdisplay ln2 20)")
    (sdisplay ln2 20)
    (println)
    )

(run7)

