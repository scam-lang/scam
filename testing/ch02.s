(define x 1000000)
(define f (/ 1.0 16))
(define (smaller amount fraction)
	(inspect (* amount fraction))
	)

;/****************************************************/

(define (test)
    (println "Chapter 2 test...\n")
    (inspect x)
    (inspect f)
    (print "smaller(x,f)...\n    ")
    (smaller x f)
    )

(test)
