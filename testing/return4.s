(define (return2 x)
	(return (return (* 2 x)))
	)

(define (f1 x)
	(return2 (+ x 3))
	0
	)
	
(define (t1)
	(if (= (f1 1) 8)
		(println "t1 success")
		(println "t1 failure")
		)
	)
	
(define (return3 x)
	(+ 1 1)
	(return 
		(begin
			(+ 1 1)
			(return (- x 1))
			10
			)
		)
	)

(define (f2 x)
	(return3 1)
	100
	)
	
(define (t2)
	(if (= (f2 1) 0)
		(println "t2 success")
		(println "t2 failure")
		)
	)
	
(define (f3)
	(define i 10)
	(for 'ok (> i 0) (-- i)
		(+ 1 1)
		(if (= i 3)
			(return2 50)
			1)
		2)
	3)

(define (f4)
	(define i 10)
    (while (> i 0)
        (+ 1 1)
        (if (= i 3)
			(return2 50)
			1)
        (-- i)
		2)
	3)
	
(define (t3)
	(if (= (f3) 100)
		(println "t3 success")
		(println "t3 failure")
		)
	)
	
(define (t4)
	(if (= (f4) 100)
		(println "t4 success")
		(println "t4 failure")
		)
	)
	
(define (f5 x)
	(define a 
		((lambda (y) (return y)) x)
		)
	(return a)
	)

(define (t5)
	(if (= (f5 10) 10)
		(begin (println "t5 success") (return 0))
		0)
	(println "t5 failure")
	)
	
(define (f6 x)
	(return (begin (return (begin (return x) 0)) 0))
	0
	)
	
(define (t6)
	(if (= ((lambda (x) ((lambda (x) (f6 x)) x)) 10) 10)
		(begin (println "t6 success") (return 0))
		0)
	(println "t6 failure")
	)
	
(println "Begining stacking return tests")

(t1)
(t2)
(t3)
(t4)
(t5)
(t6)

(println "Stacking return testing complete")