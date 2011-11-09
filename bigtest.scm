(include "assign2.scm")
(include "run.scm")
(include "pretty.lib")
(define jcl
    (scope
        (include "assign2.scm")
        this
        )
    )
;(assign (dot jcl __context) __context) ;/skip over current context

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
        run8
        run9
        ;run10
        )
    )

(define (runs #)
    (define s)
    (for-each2 s exercises
        ((eval s #))
        ((eval (symbol (string+ "jcl-" (string s))) #))
        )
    )


(define outputFile "report")

(define (prettyPrinter items env)
    (cond
        ((null? items) nil)
        (else
            (display "your ")
            (display (car items))
            (display " function:\n\n")
            (prettyIndent (eval (car items) env) "    ")
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
    (println "my tests:\n")
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
    (println "my tests:\n")
    (iter items)
    )

(define (functionTester items env @)
    (define sep (if (valid? @) (car @) " "))
    (define (iter stuff)
        (define answer
             (if (null? stuff)
                 nil
                 (catch (eval (car stuff) env))
                 ;(eval (car stuff) env)
                 )
             )
        (cond
            ((null? stuff) nil)
            ((string? (car stuff))
                (println (car stuff))
                (iter (cdr stuff))
                )
            (else
                (println "test: " (car stuff) " is:" sep answer)
                (iter (cdr stuff))
                )
            )
        )
    (println "my tests:\n")
    (iter items)
    )

(define (grep intro command)
    (print intro)
    (println)
    (system (string+ "grep " command " assign\*.scm > .grep.out"))
    (setPort (open ".grep.out" 'read))
    (define line (readLine))
    (while (not (eof?))
        (println line)
        (set! line (readLine))
        )
    (println)
    (close (getInputPort))
    )

(define (go)
    (run exercises outputFile)
    )

;;;;; exercise 1 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (jcl-run1)
    (define jcl+ (dot jcl +))
    (define jcl-polar (dot jcl polar))
    (define jcl-rectangular (dot jcl rectangular))
    (define tests '(
        (+ 2 3)
        (jcl+ 2 3)
        (+ 2 3.3)
        (+ (polar 1 .707) (polar 1 .707))
        (jcl+ (jcl-polar 1 .707) (jcl-polar 1 .707))
        (+ (polar 3 1.423) (polar 7 2.453))
        (jcl+ (jcl-polar 3 1.423) (jcl-polar 7 2.453))
        (+ (rectangular 3 1.423) (rectangular 7 2.453))
        (jcl+ (jcl-rectangular 3 1.423) (jcl-rectangular 7 2.453))
        "\nextra credit\n"
        (+ (rectangular 3 1.423) 2)
        (jcl+ (jcl-rectangular 3 1.423) 2)
        (+ 2 (rectangular 3 1.423))
        (jcl+ 2 (jcl-rectangular 3 1.423))
        ))

    (prettyPrinter '(polar rectangular +) __context)

    (functionTester tests this "\n    ")
    )

;;;;; exercise 2 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (jcl-run2)
    (define tests '(
        (translate (add two three))
        (translate (add zero five))
        (translate (add five zero))
        (translate (add three six))
        (translate (add five four))
        "\nextra credit\n"
        (translate (add nine nine))
        " "
        "RUN FUNCTION SHOULD HAVE EXPLANATION"
        ))

    (prettyPrinter '(one two nine add) __context)

    (functionTester tests this)
    )

;;;;; exercise 3 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (jcl-run3)
    (define tests '(
        (define->lambda '(define (plus a b) (+ a b)))
        (define->lambda '(define (plus a b) (inspect a) (+ a b)))
        (define->lambda '(define (double a) (+ a a)))
        (define->lambda '(define (double a) (inspect a) (+ a a)))
        (define->lambda '(define (sum a b c) (+ a b c)))
        (define->lambda '(define (sum a b c) (inspect a) (+ a b c)))
        ))

    (prettyPrinter '(define->lambda) __context)

    (functionTester tests this "\n    ")
    )

;;;;; exercise 4 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (jcl-run4)
    (define tests '(
        "car"
        ((extractor 0) '((1 2) 3))
        "cdr"
        ((extractor 1) '((1 2) 3))
        "cadr"
        ((extractor 1 0) '((1 2) 3))
        "cdar"
        ((extractor 0 1) '((1 2) 3))
        "cadar"
        ((extractor 0 1 0) '((1 2) 3))
        "caaddddr"
        ((extractor 1 1 1 1 0 0) '((1) (2) (3) (4) (5) (6)))
        ))

    (prettyPrinter '(extractor) __context)

    (functionTester tests this)
    )

;;;;; exercise 5 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (jcl-run5)
    (define jcl-dot-product (dot jcl dot-product))
    (define jcl-accumulate-n (dot jcl accumulate-n))
    (define jcl-matrix-*-vector (dot jcl matrix-*-vector))
    (define jcl-matrix-*-matrix (dot jcl matrix-*-matrix))
    (define tests '(
        (dot-product '(1 2 3 4) '(5 6 7 8))
        (jcl-dot-product '(1 2 3 4) '(5 6 7 8))
        (dot-product '(1 2) '(3 4))
        (jcl-dot-product '(1 2) '(3 4))
        (accumulate-n cons nil '((1 2 3) (4 5 6)))
        (jcl-accumulate-n cons nil '((1 2 3) (4 5 6)))
        (matrix-*-vector '((1 2 3) (4 5 6)) '(3 4))
        (jcl-matrix-*-vector '((1 2 3) (4 5 6)) '(3 4))
        (matrix-*-matrix '((1 2 3)(4 5 6)(7 8 9)) '((1 0 0)(0 1 0)(0 0 1)))
        (jcl-matrix-*-matrix '((1 2 3)(4 5 6)(7 8 9)) '((1 0 0)(0 1 0)(0 0 1)))
        (matrix-*-matrix '((1 2 3)(4 5 6)(7 8 9)) '((1 2 1)(2 1 2)(1 3 1)))
        (jcl-matrix-*-matrix '((1 2 3)(4 5 6)(7 8 9)) '((1 2 1)(2 1 2)(1 3 1)))
        (matrix-*-matrix '((1 2 3)(4 5 6)) '((1 2)(3 4)(5 6)))
        (jcl-matrix-*-matrix '((1 2 3)(4 5 6)) '((1 2)(3 4)(5 6)))
        ))

    (prettyPrinter '(dot-product accumulate-n matrix-*-vector matrix-*-matrix) __context)

    (functionTester tests this "\n    ")
    )

;;;;; exercise 6 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (jcl-run6)
    (define jcl-infix->postfix (dot jcl infix->postfix))
    (define tests '(
        (infix->postfix '(1 + 3))
        (jcl-infix->postfix '(1 + 3))
        (infix->postfix '(1 + 3 - 4 * 5 / 6 ^ 7))
        (jcl-infix->postfix '(1 + 3 - 4 * 5 / 6 ^ 7))
        (infix->postfix '(1 + 3 * 4 ^ 5 - 6 / 7))
        (jcl-infix->postfix '(1 + 3 * 4 ^ 5 - 6 / 7))
        (infix->postfix '(a ^ b / c * d - e + f))
        (jcl-infix->postfix '(a ^ b / c * d - e + f))
        (infix->postfix '(1 + 3 * 2 - 4 ^ 5 / 6 ^ 7 - 8 * 9 + 0))
        (jcl-infix->postfix '(1 + 3 * 2 - 4 ^ 5 / 6 ^ 7 - 8 * 9 + 0))
        ))

    (prettyPrinter '(infix->postfix) __context)

    (functionTester tests this "\n    ")
    )


;;;;; exercise 7 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (jcl-run7)
    (define tests '(
        (powerSet nil)
        (powerSet '(a b))
        (powerSet '(a b c))
        (powerSet '(c b a))
        (powerSet '(z y x w))
        ))

    (prettyPrinter '(powerSet) __context)

    (functionTester tests this "\n    ")
    )


;;;;; exercise 8 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (jcl-run8)
    (define jcl-depthStat (dot jcl depthStat))
    (define t (treeNode 2 (treeNode 1 nil (treeNode 4 nil nil)) (treeNode 0 nil nil)))
    (define tests '(
        t
        (depthStat t)
        (jcl-depthStat t)
        (depthStat (treeNode 5 t t))
        (jcl-depthStat (treeNode 5 t t))
        (depthStat (treeNode 5 (treeNode 6 t nil) t))
        (jcl-depthStat (treeNode 5 (treeNode 6 t nil) t))
        ))

    (prettyPrinter '(depthStat) __context)

    (functionTester tests this)
    )

;;;;; exercise 9 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (jcl-run9)
    (define (stringify x) ((dot x toString)))
    (define jcl-Polar (dot jcl Polar))
    (define jcl-Rectangular (dot jcl Rectangular))
    (define jcl-Real (dot jcl Real))
    (define jcl-Integer (dot jcl Integer))
    (define jcl-P (jcl-Polar 3 1.43))
    (define jcl-R (jcl-Rectangular 3 1.43))
    (define jcl-r (jcl-Real 1.43))
    (define jcl-i (jcl-Integer 4))
    (define P (Polar 3 1.43))
    (define R (Rectangular 3 1.43))
    (define r (Real 1.43))
    (define i (Integer 4))
    (define tests '(
        (stringify P)
        (stringify R)
        (stringify r)
        (stringify i)
        "\nconvert\n"
        (stringify ((dot ((dot P convert)) convert)))
        (stringify ((dot ((dot R convert)) convert)))
        "\npromote\n"
        (stringify ((dot r promote)))
        (stringify ((dot i promote)))
        (stringify ((dot jcl-r promote)))
        (stringify ((dot jcl-i promote)))
        "\ndemote\n"
        (stringify ((dot P demote)))
        (stringify ((dot R demote)))
        (stringify ((dot r demote)))
        (stringify ((dot jcl-P demote)))
        (stringify ((dot jcl-R demote)))
        (stringify ((dot jcl-r demote)))
        "\nadd Polar + ?\n"
        (stringify ((dot P add) P))
        (stringify ((dot P add) R))
        (stringify ((dot jcl-P add) jcl-P))
        (stringify ((dot jcl-P add) jcl-R))
        "\nadd Rectangular + ?\n"
        (stringify ((dot R add) P))
        (stringify ((dot R add) R))
        (stringify ((dot jcl-R add) jcl-P))
        (stringify ((dot jcl-R add) jcl-R))
        "\nadd Real + ?\n"
        (stringify ((dot r add) r))
        (stringify ((dot jcl-r add) jcl-r))
        "\nadd Integer + ?\n"
        (stringify ((dot i add) i))
        (stringify ((dot jcl-i add) jcl-i))
        "\n+\n"
        (+ 2 3)
        (+ 2.2 3)
        (+ (rectangular 2 2) (rectangular 3 3))
        (+ (rectangular 2 2) (rectangular 3 3))
        (+ (polar 1 .707) (polar 1 .707))
        "\n+ (with objects)\n"
        (stringify (+ (Rectangular 2 2) (Rectangular 3 3)))
        (stringify (+ (Polar 1 .707) (Polar 1 .707)))
        (stringify (+ (Real .707) (Real 1.203)))
        (stringify (+ (Integer 2) (Integer 3)))
        "\nextra credit\n"
        (stringify (+ (Rectangular 2 2) (Real 3.0)))
        (stringify (+ (Real 3.0) (Rectangular 2 2)))
        (stringify (+ (Real 3.0) (Integer 2)))
        (stringify (+ (Integer 1) (Polar 1 0)))
        ))

    (prettyPrinter '(+ Integer Real Rectangular Polar) __context)

    (functionTester tests this)
    )

(define (jcl-run10)
    (define (stringify x) ((dot x toString)))
    (define P (Polar 3 1.43))
    (define R (Rectangular 3 1.43))
    (define r (Real 1.43))
    (define i (Integer 4))
    (define tests '(
        (stringify P)
        (stringify R)
        (stringify r)
        (stringify i)
        "coerce"
        (stringify (coerce (coerce P 'Rectangular) 'Polar))
        (stringify (coerce (coerce R 'Polar) 'Rectangular))
        (stringify (coerce R 'Real))
        (stringify (coerce r 'Rectangular))
        (stringify (coerce r 'Integer))
        (stringify (coerce i 'Real))
        (stringify (coerce i 'Rectangular))
        (stringify (coerce R 'Integer))
        (stringify (coerce i 'Rectangular))
        ))

    (prettyPrinter '(coerce) __context)

    (grep "your put commands:\n" "put")

    (functionTester tests this)
    )
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(display "\ntest-1.scm loaded successfully!\n\n")
(go)
(display "results are stored in a file named 'report'\n")
