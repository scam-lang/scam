

;(pp (eval '(begin (pp this) (eval '(begin (pp this) (return 0)) highenv)) lowenv))
;(pp (eval '(begin (pp this) (eval '(begin (pp this) (return 0)) lowenv)) highenv))

(define (helper1)
	(return 0)
	1)

(define (helper2)
	(if #t (return 0) 1)
	2)
	
(define (recur n)
	(if (> n 1) (recur (- n 1)) this)
	)

(define highenv (recur 10))
(define lowenv (recur 5))

(define (helper3)
	(eval '(begin (eval '(begin (return 0)) highenv) (return 1)) lowenv)
	)
	
(define (helper4)
	(eval '(begin (eval '(begin (return 0)) lowenv) (return 1)) highenv)
	)

(define (helper6)
	(eval '(return 0) this)
	1)
	
(define (helper7)
	(eval '(eval '(begin (eval '(return 0) highenv) 1) lowenv) highenv))
	
(define (r) (return 0))

	
(define (test1)
	(if (= 0 (helper1)) (println "test1 successful") (println "test1 unsuccessful")))

(define (test2)
	(if (= 0 (helper2)) (println "test2 successful") (println "test2 unsuccessful")))
	
(define (test3)
	(if (= 1 (helper3)) (println "test3 successful") (println "test3 unsuccessful")))
	
(define (test4)
	(if (= 0 (helper4)) (println "test4 successful") (println "test4 unsuccessful")))
	
(define correct 0)
(define (test5)
	(eval '(evalList '((r) (set 'correct 1)) lowenv) highenv)
	(if (= 1 correct) (println "test5 successful") (println "test5 unsuccessful"))
	)

(define (test6)
	(if (= 0 (helper6)) (println "test6 successful") (println "test6 unsuccessful")))
	
(define (test7)
	(if (= 1 (helper7)) (println "test7 successful") (println "test7 unsuccessful")))

(println "Begining return tests, 7 tests")	
(test1)
(test2)
(test3)
(test4)
(test5)
(test6)
(test7)
(println "Return tests completed")