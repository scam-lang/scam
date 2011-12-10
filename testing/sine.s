(define amplitude 1)
(define frequency 1)
(define shift 0)

(define width)
(define low -2.0)
(define high 10.0)
(define step .25)

(define x)

(define (sineWave a f s)
    (lambda (x)
        (* a  (sin (+ (* f x) s)))
        )
    )

(define s (sineWave amplitude frequency shift))

(define port (open "sine.dat" 'write))
(setPort port)

(println "# x\ty")
(for (set 'x low) (<= x high) (+= x step)
    (println x "\t" (s x))
    )

(close port)

(system "cat sine.dat")
