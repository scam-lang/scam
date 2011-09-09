(include "assign1.scm")
(include "run.scm")
(include "pretty.lib")

(define (inspect # $item)
    (define result (catch (eval $item #)))
    (println $item " is " result)
    )

(define exercises
       '(
        run1
        run2
        run3
        run4
        run5
        run6
        run7
        ;run8
        )
    )

(define outputFile "report")

(define (prettyPrinter items env)
    (cond
        ((null? items) nil)
        (else
            (println "your " (car items) " function:\n")
            (ppCode (eval (car items) env) "    ")
            (println "\n")
            (prettyPrinter (cdr items) env)
            )
        )
    )

(define (silentTester items env)
    (define (iter stuff)
        (cond
            ((null? stuff) nil)
            ((string? (car stuff))
                (println (car stuff))
                (iter (cdr stuff))
                )
            (else
                (catch (eval (car stuff) env))
                (iter (cdr stuff))
                )
            )
        )
    (iter items)
    )

(define (procedureTester items env)
    (define (iter stuff)
        (cond
            ((null? stuff) nil)
            ((string? (car stuff))
                (println (car stuff))
                (iter (cdr stuff))
                )
            (else
                (println "test: " (car stuff))
                (catch (eval (car stuff) env))
                (iter (cdr stuff))
                )
            )
        )
    (iter items)
    )

(define (functionTester items env)
    (define (iter stuff)
        (define answer
             (if (null? stuff)
                 nil
                 ;(catch (eval (car stuff) env))
                 (eval (car stuff) env)
                 )
             )
        (cond
            ((null? stuff) nil)
            ((string? (car stuff))
                (println (car stuff))
                (iter (cdr stuff))
                )
            (else
                (println "test: " (car stuff) " is " answer)
                (iter (cdr stuff))
                )
            )
        )
    (iter items)
    )

(define (go)
    (run exercises outputFile)
    )

;;;;; exercise 1 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (jcl-run1)
    (define tests '(
        "no testing needed"
        ))

    (prettyPrinter '() __context)

    (functionTester tests this)
    )

;;;;; exercise 2 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (jcl-run2)
    (define tests '(
        (zeno_cost 20 150 0.25)
        ))

    (prettyPrinter '(zeno_cost) __context)

    (functionTester tests this)
    )

;;;;; exercise 3 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (jcl-run3)
    (define tests '(
        (min8 0 0 0 0 0 0 0 0)
        ))

    (prettyPrinter '(min8) __context)

    (functionTester tests this)
    )

;;;;; exercise 4 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (jcl-run4)
    (define tests '(
        (root3 8.0)
        (root3 27.0)
        ))

    (prettyPrinter '(root3) __context)

    (functionTester tests this)
    )

;;;;; exercise 5 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (jcl-run5)
    (define tests '(
        (pt 8)
        (pt 15)
        ))

    (prettyPrinter '(pt) __context)

    (procedureTester tests this)
    )

;;;;; exercise 6 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (jcl-run6)
    (define tests '(
        (zorp 0 (lambda (x) (* x x)))
        (zorp 1 (lambda (x) (* x x)))
        (zorp 2 (lambda (x) (* x x)))
        (zorp 3 (lambda (x) (* x x)))
        (zorp 4 (lambda (x) (* x x)))
        ))

    (prettyPrinter '(zorp) __context)

    (functionTester tests this)
    )


;;;;; exercise 7 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (jcl-run7)
    (define tests '(
        (halve 2)
        (halve 16)
        (square 3)
        (square 5)
        (babyl* 9 8)
        (babyl* 8 9)
        ))

    (prettyPrinter '(halve square babyl*) __context)

    (functionTester tests this)
    )

;;;;; exercise 8 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (jcl-run8)
    (define (poly x) (* 4 x x x x))
    (define tests '(
        (sqrt 16)
        (fixed-point cos 1.0)
        ((iterative-improve (lambda (x) (< x 1)) (lambda (x) (/ x 2.0))) 32) 
        ))

    (prettyPrinter '(iterative-improve sqrt fixed-point) (lambda () #t))

    (functionTester tests (lambda () #t))
    )

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(display "\ntest-1.scm loaded successfully!\n\n")
(go)
(display "results are stored in a file named 'report'\n")
