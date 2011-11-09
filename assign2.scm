;by Kaleb Williams and Yujun Liu
(include "reflection.lib")

;*****************************Problem 1*****************************
(define (rectangular x y)
	(list (cons 'type 2) (cons 'x x) (cons 'y y)))

(define (polar mag theta)
	(list (cons 'type 3) (cons 'mag mag) (cons 'theta theta)))

(define (printRect rect)
	(print (getX rect) "+" (getY rect) "i"))

(define (printPol pol)
	(print (getMag pol) "|" (getTheta pol)))

(define (getX rect)
	(cdr (assoc 'x rect)))

(define (getY rect)
	(cdr (assoc 'y rect)))

(define (getMag pol)
	(cdr (assoc 'mag pol)))

(define (getTheta pol)
	(cdr (assoc 'theta pol)))
; + is redefined in Problem 9

;*****************************Problem 2*****************************
(define (inc x) (cons 'x x))
(define base ())

(define (increment number)
	(define (next-higher-number incrementer)
	(define (resolve base)
		(incrementer ((number incrementer) base))
		)
	resolve
	)
	next-higher-number
	)

(define (identity x) x)

(define (zero arg) identity)
(define (one incrementer) ((increment zero) incrementer))
(define (two incrementer) ((increment one) incrementer))
(define (three incrementer) ((increment two) incrementer))
(define (four incrementer) ((increment three) incrementer))
(define (five incrementer) ((increment four) incrementer))
(define (six incrementer) ((increment five) incrementer))
(define (seven incrementer) ((increment six) incrementer))
(define (eight incrementer) ((increment seven) incrementer))
(define (nine incrementer) ((increment eight) incrementer))

(define numbers
	(list
		(list ((zero inc) base) 'zero)
		(list ((one inc) base) 'one)
		(list ((two inc) base) 'two)
		(list ((three inc) base) 'three)
		(list ((four inc) base) 'four)
		(list ((five inc) base) 'five)
		(list ((six inc) base) 'six)
		(list ((seven inc) base) 'seven)
		(list ((eight inc) base) 'eight)
		(list ((nine inc) base) 'nine)
		)
	)

(define (add num1 num2)
	(append ((num1 inc) base) ((num2 inc) base)))

(define (translate num)
	(cadr (assoc num numbers)))

;*****************************Problem 3*****************************
(define (define->lambda sc)
	(define part1 (car sc))
	(define part2 (car (cadr sc)))
	(define part3 (caddr sc))
	(list part1 part2 (list 'lambda (cdr (cadr sc)) part3))	
	)

;*****************************Problem 4*****************************
(define (extractor @)
	(define (recur i fxn)
		(cond
			((= i (length @))
				fxn)
			(else
				(if (= (getElement @ i) 0) 
					(recur (+ i 1) (car fxn))
					(recur (+ i 1) (cdr fxn))))))
	(lambda (x) (recur 0 x)))

;*****************************Problem 5*****************************
(define (accumulate op init items)
        (cond
                ((null? items) init)
                (else (op (car items) (accumulate op init (cdr items))))))

(define (accumulate-n op init seqs)
        (if (null? (car seqs))
                nil
                (cons (accumulate op init (map (lambda (x) (car x)) seqs))
                        (accumulate-n op init (map (lambda (x) (cdr x)) seqs)))))

(define (dot-product v w)
        (accumulate + 0 (map * v w)))

(define (matrix-*-vector m v)
	(let ((rows (transpose m)))
	        (map (lambda (row) (dot-product v row)) rows)))

(define (transpose mat)
        (accumulate-n cons nil mat))

(define (matrix-*-matrix m n)
	(map (lambda (col) (matrix-*-vector m col)) n))

;*****************************Problem 6*****************************
(define (infix->postfix expr)
        ;expr is a list representing the expression
        (define (op? sym)
                (or (eq? sym '+) (eq? sym '-) (eq? sym '*) (eq? sym '/) (eq? sym '^)))

        (define (lowerPriority compare lst)
                (cond
                        ((null? lst) #f)
                        ((eq? (car lst) compare) #t)
                        (else (lowerPriority compare (cdr lst)))))

        (define (opRecur ops current output)
                (cond
                        ((null? ops)
                                (cons output (append (list current) ops)))
                        ((eq? current '^)
                                (cons output (append (list current) ops)))
                        ((eq? current '/)
                                (if (lowerPriority (car ops) '(^))
                                        (opRecur (cdr ops) current (append output (list (car ops))))
                                        (cons output (append (list current) ops))))
                        ((eq? current '*)
                                (if (lowerPriority (car ops) '(/ ^))
                                        (opRecur (cdr ops) current (append output (list (car ops))))
                                        (cons output (append (list current) ops))))
                        ((eq? current '-)
                                (if (lowerPriority (car ops) '(* / ^))
                                        (opRecur (cdr ops) current (append output (list (car ops))))
                                        (cons output (append (list current) ops))))
                        ((eq? current '+)
                                (if (lowerPriority (car ops) '(- * / ^))
                                        (opRecur (cdr ops) current (append output (list (car ops))))
                                        (cons output (append (list current) ops))))))

        (define (finishOps ops output)
                (cond
                        ((null? ops)
                                output)
                        (else
                                (finishOps (cdr ops) (append output (list (car ops)))))))


        (define (inputRecur input output ops)
                (cond
                        ((null? input)
                                (finishOps ops output))
                        ((op? (car input))
                                (define cell (opRecur ops (car input) output))
                                (inputRecur (cdr input) (car cell) (cdr cell)))
                        (else
                                (inputRecur (cdr input) (append output (list (car input))) ops))))

        (inputRecur expr () ()))

;*****************************Problem 7*****************************
(define (powerSet originalSet)
        (define (powerRecur set)
                (if (null? set) (list '())
                        (let ((set2 (powerRecur (cdr set))))
				(append (map (lambda(x) (cons (car set) x))set2) set2))))
        (define unfilSet (powerRecur originalSet))
        (define (filterRecur filSet lnth)
                (cond
                        ((= lnth (length unfilSet))
                                (append filSet (powerKeep helper unfilSet lnth)))
                        (else
                                (filterRecur (append filSet (powerKeep helper unfilSet lnth)) (+ lnth 1)))))
        (cons '() (filterRecur nil 0)))


(define (powerKeep p? items lngth)
        (cond
                ((null? items) nil)
                ((p? (car items) lngth)
                        (cons (car items) (powerKeep p? (cdr items) lngth)))
                (else (powerKeep p? (cdr items) lngth))))

(define (helper x lngth)
        (if (eq? x nil)
                #f
                (= (length x) lngth)))

;*****************************Problem 8*****************************
(define (newTree) ())

(define (treeNode value left right)
        (list 'binaryTree value left right)
        )

(define (enumerate-tree tree depth)
        (cond ((null? tree)
                        nil)
                ((and (null? (caddr tree)) (null? (cadddr tree)))
                        (list (cons (cadr tree) depth)))
                (else
                        (append (enumerate-tree (caddr tree) (+ depth 1)) (enumerate-tree (cadddr tree) (+ depth 1))))))

(define (keep p? items)
        (cond
                ((null? items) nil)
                ((p? (car items))
                        (cons (car items) (keep p? (cdr items))))
                (else (keep p? (cdr items)))))

(define (accumulate op init items)
        (cond
                ((null? items) init)
                (else (op (car items) (accumulate op init (cdr items))))))

(define (depthStat tree)
        (define enum (enumerate-tree tree 0))
        (define mapped (map cdr enum))
        (define accumulated (accumulate + 0 mapped))
        (/ (real accumulated) (length mapped)))

;*****************************Problem 9*****************************
;   Also contains + for Problem 1

(define (Integer val)
        (define (type)
                0)
        (define (add val2)
                (Integer (+ val ((dot val2 value)))))
        (define (promote)
                (Real (real val)))
        (define (demote)
                (Integer val))
        (define (toString)
                (string val))
        (define (value)
                val)
        ;(define (dispatch msg)
        ;        (cond   ((eq? msg 'add) add)
        ;                ((eq? msg 'promote) promote)
        ;                ((eq? msg 'demote) demote)
        ;                ((eq? msg 'toString) toString)
        ;                ((eq? msg 'value) value)
        ;                ((eq? msg 'type) type)))
        ;dispatch)
	this)

(define (Real val)
        (define (type)
                1)
        (define (add val2)
                (Real (+ val ((dot val2 value)))))
        (define (promote)
                (Rectangular val 0))
        (define (demote)
                (Integer (int val)))
        (define (toString)
                (string val))
        (define (value)
                (real val))
        ;(define (dispatch msg)
        ;        (cond   ((eq? msg 'add) add)
        ;                ((eq? msg 'promote) promote)
        ;                ((eq? msg 'demote) demote)
        ;                ((eq? msg 'toString) toString)
        ;                ((eq? msg 'value) value)
        ;                ((eq? msg 'type) type)))
        ;dispatch)
	this)

(define (Rectangular num imag)
        (define (type)
                2)
        (define (add num2)
                (Rectangular (+ num (car((dot num2 value)))) (+ imag (cdr ((dot num2 value))))))
        (define (promote)
                (Rectangular num imag))
        (define (demote)
                (Real (real num)))
        (define (toString)
                (string+ (string num) "+" (string imag) "i"))
        (define (value)
                (cons num imag))
        (define (convert)
                (Polar (sqrt (+ (* num num) (* imag imag))) (atan imag num)))
        ;(define (dispatch msg)
        ;        (cond   ((eq? msg 'add) add)
        ;                ((eq? msg 'promote) promote)
        ;                ((eq? msg 'demote) demote)
        ;                ((eq? msg 'toString) toString)
        ;                ((eq? msg 'value) value)
        ;                ((eq? msg 'convert) convert)
        ;                ((eq? msg 'type) type)))
        ;dispatch)
	this)

(define (Polar mag theta)
        (define (type)
                3)
        (define (convert)
                (Rectangular (* mag (cos theta)) (* mag (sin theta))))
        (define (add num2)
                (define rect1 (convert))
                (define rect2 ((dot num2 convert)))
                ((dot ((dot rect1 add) rect2) convert)))
        (define (promote)
                (Polar mag theta))
        (define (demote)
                ((dot (convert) demote)))
        (define (toString)
                (string+ (string mag) "|" (string theta)))
        (define (value)
         	(cons mag theta))
        ;(define (dispatch msg)
        ;        (cond   ((eq? msg 'add) add)
        ;                ((eq? msg 'promote) promote)
        ;                ((eq? msg 'demote) demote)
        ;                ((eq? msg 'toString) toString)
        ;                ((eq? msg 'value) value)
        ;                ((eq? msg 'convert) convert)
        ;                ((eq? msg 'type) type)))
        ;dispatch)
	this)

(redefine (+ num1 num2)
	(define (object? a)
		(cond 
			((pair? a)(eq? (car num1) 'object))
			(else #f)))
        (cond
		((or (integer? num1) (real? num1) (not (object? num1)))
                        ;Our type hierarchy:
                        ;       polar = 3
                        ;       rect = 2
                        ;       real = 1
                        ;       integer = 0
                        (define (type num)
                                (cond
                                        ((real? num) 1)
                                        ((integer? num) 0)
                                        (else (cdr (assoc 'type num)))))

                        (define (intToReal int)
                                (real int))

                        (define (realToRect r)
                                (rectangular r 0))

                        (define (rectToPol rect)
                                (define x (getX rect))
                                (define y (getY rect))
                                (polar (sqrt (+ (* x x) (* y y))) (atan y x)))

                        (define (polToRect pol)
                                (define mag (getMag pol))
                                (define theta (getTheta pol))
                                (rectangular (* mag (cos theta)) (* mag (sin theta))))

                        (define promoteList (list (cons 0 intToReal) (cons 1 realToRect) (cons 2 rectToPol)))

                        (define (promote num)
                                ((cdr (assoc (type num) promoteList)) num))

                        (define (rect+ rect1 rect2)
                                (define x (+ (getX rect1) (getX rect2)))
                                (define y (+ (getY rect1) (getY rect2)))
                                (rectangular x y))

                        (define (pol+ pol1 pol2)
                                (define rect1 (polToRect pol1))
                                (define rect2 (polToRect pol2))
                                (rectToPol (rect+ rect1 rect2)))

                        (define plusList (list (cons 0 (prior)) (cons 1 (prior)) (cons 2 rect+) (cons 3 pol+)))
                        (cond
                                ((and (= (type num1) 2) (= (type num2) 3))
                                        (+ num1 (polToRect num2)))
                                ((< (type num1) (type num2))
                                        (cond
                                                ((or (= (type num2) 2) (= (type num2) 3))
                                                        (+ num2 (promote num1)))
                                                (else
                                                        (+ (promote num1) num2))))
                                ((= (type num1) (type num2))
                                        ((cdr (assoc (type num1) plusList)) num1 num2))
                                (else
                                        (+ num1 (promote num2)))))
		(else
                        (cond
                                ((or (and (= ((dot num1 type)) 2) (= ((dot num2 type)) 3)) (and (= ((dot num1 type)) 3) (= ((dot num2 type)) 2)))
                                        (+ num1 ((dot num2 convert))))
                                ((< ((dot num1 type)) ((dot num2 type)))
                                        (cond
                                                ((or (= ((dot num2 type)) 2) (= ((dot num2 type)) 3))
                                                        (+ num2 ((dot num1 promote))))
                                                (else
                                                        (+ ((dot num1 promote)) num2))))
                                ((> ((dot num1 type)) ((dot num2 type)))
                                        (+ num1 ((dot num2 promote))))
                                (else
                                        ((dot num1 add) num2))))))


(define (coerce num resultType)
        (cond
                ((= ((dot num type)) 0)
                        ((get (append '(Integer) (list resultType))) num))
                ((= ((dot num type)) 1)
                        ((get (append '(Real) (list resultType))) num))
                ((= ((dot num type)) 2)
                        ((get (append '(Rectangular) (list resultType))) num))
                ((= ((dot num type)) 3)
                        ((get (append '(Polar) (list resultType))) num))))


;********************************run********************************
(define (run1)
	(println "Problem 1:")
	(define rect1 (rectangular 1 3))
	(define rect2 (rectangular 6 10.3))
	(define pol1 (polar 5 3.14))
	(define pol2 (polar 6.2 6))
	(println "The sum of 22 and 999 is " (+ 22 999) " [Should be 1021]\n")
	(println "The sum of 1.3 and 6.2 is " (+ 1.3 6.2) " [Should be 7.5]\n")
	(print "The sum of the rectangles ") (printRect rect1) (print " and ") (printRect rect2) (print " is ") (printRect (+ rect1 rect2)) (print " [Should be ") (printRect (rectangular 7 13.3)) (println "]\n")
	(print "The sum of the polars ") (printPol pol1) (print " and ") (printPol pol2) (print " is ") (printPol (+ pol1 pol2)) (print " [Should be ") (printPol (polar 1.97026064 -1.065891804)) (println "]\n")
	(print "The sum of the rectangle ") (printRect rect1) (print " and the polar ") (printPol pol2) (print " is ") (printRect (+ rect1 pol2)) (print " [Should be ") (printRect (rectangular 6.953055777 1.267623911)) (println "]\n")
	(print "The sum of the polar ") (printPol pol1) (print " and the rectangle ") (printRect rect2) (print " is ") (printPol (+ pol1 rect2)) (print " [Should be ") (printPol (polar 10.35635647 1.474085976)) (println "]\n")
	(print "The sum of 7 and the polar ") (printPol pol2) (print " is ") (printPol (+ 7 pol2)) (print " [Should be ") (printPol (polar 13.06838861 -0.1329536841)) (println "]\n")
	(println))

(define (run2)
	(println "Problem 2:")
	(println 
"increment returns the function next-higher-number, which returns the function 
resolve, which returns (incrementer ((number incrementer) base)). So increment 
zero ultimately returns (incrementer ((zero incrementer) base)) (after a couple calls). Since the 
function zero returns the identity function, this reduces to (incrementer 
(identity base)). The identity function returns whatever it is given, so 
(identity base) resolves to base. So (incrementer (identity base)) resolves to 
(incrementer base).
	one returns the function resolver, which returns incrementer
 base. So one incrementer will ultimately return incrementer base. So they are 
equivalent.\n")
	(println "(add two three) is " (translate (add two three)))
	(println "(add six three) is " (translate (add six three)))
	(println "(add zero zero) is " (translate (add zero zero)) "\n")
	(println))

(define (run3)
	(println "Problem 3:")
	(println "(define->lambda (quote (define (plus a b) (+ a b)))) evaluates to " (define->lambda (quote (define (plus a b) (+ a b)))) "\n")
	(println))

(define (run4)
	(println "Problem 4:")
	(println "((extractor 1 1 0) '(1 2 3 4 5)) returns " ((extractor 1 1 0) '(1 2 3 4 5)) " [Should return 3]\n")
	(println "((extractor 1 1 1 1 1 1 1 0) '(1 2 3 4 5 6 7 8 9)) returns " ((extractor 1 1 1 1 1 1 1 0) '(1 2 3 4 5 6 7 8 9)) " [Should return 8]\n")
	(println "((extractor 0 1 0) (cons (cons 1 (cons 2 3)) (cons 4 5))) returns " ((extractor 0 1 0) (cons (cons 1 (cons 2 3)) (cons 4 5))) " [Should return 2]\n")
	(println))

(define (run5)
	(println "Problem 5:")
	(println "The dot-product of the vectors (1 2 3) and (4 5 6) is " (dot-product (list 1 2 3) (list 4 5 6)) " [Should be 32]\n")
	(println "The matrix-*-vector product of the matrix ((1 0) (-1 -3) (2 1)) and the vector (2 1 0) is " (matrix-*-vector (list (list 1 0) (list -1 -3) (list 2 1)) (list 2 1 0)) " [Should be (1 -3)]\n")
	(println "The matrix-*-matrix product of ((0 -4)(4 -3)(-2 0)) and ((0 1 2)(1 -1 3)) is " (matrix-*-matrix (list (list 0 -4) (list 4 -3) (list -2 0))(list (list 0 1 2) (list 1 -1 3))) " [Should be ((0 -3)(-10 -1))]\n")
	(println "transposing matrix ((1 5 9)(22 44 88)(33 76 0)) results in " (transpose '((1 5 9) (22 44 88) (33 76 0))) " [Should be ((1 22 33)(5 44 76)(9 88 0))]\n")
	(println))

(define (run6)
	(println "Problem 6:")
	(println "(infix->postfix '(2 + 3 * x ^ 5 + a)) returns " (infix->postfix '(2 + 3 * x ^ 5 + a)) " [Should be (2 3 x 5 ^ * a + +)]\n")
	(println "(infix->postfix '(5 - 2 + 1 * 3 ^ apple - 1)) returns " (infix->postfix '(5 - 2 + 1 * 3 ^ apple - 1)) " [Should be (5 2 - 1 3 apple ^ * 1 - +)]\n")
	(println))

(define (run7)
	(println "Problem 7:")
	(println "(powerSet '()) gives " (powerSet '()) "\n")
	(println "(powerSet '(a b c d e)) gives " (powerSet '(a b c d e)) "\n")
	(println))

(define (run8)
	(println "Problem 8:")
	(define tree1 (treeNode 5 (treeNode 6 (treeNode 4 nil nil) (treeNode 2 nil nil)) (treeNode 7 nil (treeNode 8 nil nil))))
	(define tree2 (treeNode 1 (treeNode 2 nil nil) (treeNode 3 (treeNode 4 nil nil) (treeNode 5 nil nil))))
	(println "The average depth of the tree " tree1 " is " (depthStat tree1) "\n")
	(println "The average depth of the tree " tree2 " is " (depthStat tree2) "\n")
	(println))

(define (run9)
	(println "Problem 9:")
	(define rect1 (Rectangular 1 3))
	(define rect2 (Rectangular 6 10.3))
	(define pol1 (Polar 5 3.14))
	(define pol2 (Polar 6.2 6))
	(println "The sum of 22 and 1000 is " ((dot (+ (Integer 22) (Integer 1000)) toString)) " [Should be 1021]\n")
	(println "The sum of 1.3 and 6.2 is " ((dot (+ (Real 1.3) (Real 6.2)) toString)) " [Should be 7.5]\n")
	(println "The sum of the rectangles " ((dot rect1 toString)) " and " ((dot rect2 toString)) " is " ((dot (+ rect1 rect2) toString)) " [Should be " ((dot (Rectangular 7 13.3) toString)) "]\n")
	(println "The sum of the polars " ((dot pol1 toString)) " and " ((dot pol2 toString)) " is " ((dot (+ pol1 pol2) toString)) " [Should be " ((dot (Polar 1.97026064 -1.065891804) toString)) "]\n")
	(println "The sum of the rectangle " ((dot rect1 toString)) " and the polar " ((dot pol2 toString)) " is " ((dot (+ rect1 pol2) toString)) " [Should be " ((dot (Rectangular 6.953055777 1.267623911) toString)) "]\n")
	(println "The sum of the polar " ((dot pol1 toString)) " and the rectangle " ((dot rect2 toString)) " is " ((dot (+ pol1 rect2) toString)) " [Should be " ((dot (Polar 10.35635647 1.474085976) toString)) "]\n")
	(println "The sum of 7 and the polar " ((dot pol2 toString)) " is " ((dot (+ (Integer 7) pol2) toString)) " [Should be " ((dot (Polar 13.06838861 -0.1329536841) toString)) "]\n")
	(println))

