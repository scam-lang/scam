(define (fwhile # $test $)
    (while (eval $test #)
        (evalList $ #)
        )
    )

(define (f x)
    (if (= x 0)
        (while #t
            (println "in while, returning!")
            (return 0)
            (println "after return in while [SHOULD NOT PRINT]")
            )
        (fwhile #t
            (println "in fwhile, returning!")
            (return 0)
            (println "after return in fwhile [SHOULD NOT PRINT]")
            )
        )
    (println "done with f(x) [SHOULD NOT PRINT]")
    )

(define (g)
    (define (r)
        (println "    in function r")
        (return 0)
        (println "done with r() [SHOULD NOT PRINT]")
        )
    (println "    calling function r()")
    (r)
    (println "    done calling function r()")
    (return 1)
    )

(println "before calling f(0)")
(f 0)
(println "after calling f(0)")
(println "before calling f(1)")
(f 1)
(println "after calling f(1)")
(println "before calling g()")
(g)
(println "after calling g()")
