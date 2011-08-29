(define (run1)
    (println "need lazy evaluation!")
    )

(define (run2)
    (inspect (zeno_cost 20 150 0.25))
    )

(define (zeno_cost d c f)
    (cond
        ((<= (* c f) (/ 1 12.)) c)
        ((<= (/ d 2.) (/ 1 9600.)) (+ c 5))
        (else (+ c (zeno_cost (/ d 2.) (* c f) f)))
        )
    )

(define (run3)
    (println "I'm to tired to test!")
    )

(define (min2 a b) (if (< a b) a b))
(define (min3 a b c) (if (< a b) (min2 a c) (min2 b c)))
(define (min4 a b c d) (if (< a b) (min3 a c d) (min3 b c d)))
(define (min5 a b c d e) (if (< a b) (min4 a c d e) (min4 b c d e)))
(define (min6 a b c d e f) (if (< a b) (min5 a c d e f) (min5 b c d e f)))
(define (min7 a b c d e f g) (if (< a b) (min6 a c d e f g) (min6 b c d e f g)))
(define (min8 a b c d e f g h)
    (if (< a b) (min7 a c d e f g h) (min7 b c d e f g h))
    )
        
(define (run4)
    (inspect (min8 1 2 3 4 5 6 7 8))
    (inspect (min8 2 1 3 4 5 6 7 8))
    (inspect (min8 2 3 1 4 5 6 7 8))
    )

(define (run5)
    (inspect (root3 8.0))
    (inspect (root3 27.0))
    )

(define (root3 x)
    (define (iter lower upper)
        (define guess (/ (+ upper lower) 2))
        (define try (* guess guess guess))
        (cond
            ((= try x) guess)
            ((> try x) (iter lower guess))
            (else (iter guess upper))
            )
        )
    (iter 1 x)
    )

(define (run6)
    (println "I'm to tired to test!")
    )

(define (run7)
    (println "I'm to tired to test!")
    )

(define (run8)
    (println "I'm to tired to test!")
    )

(define (run9)
    (println "I'm to tired to test!")
    )

(define (run10)
    (println "I'm to tired to test!")
    )

(run1)
(run2)
(run3)
(run4)
(run5)
(run6)
(run7)
(run8)
(run9)
(run10)
