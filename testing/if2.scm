(define if
    (scope
        (define old-if if)
        (define (new-if # test $tBranch $)
            (pass old-if # test $tBranch $)
            )
        new-if
        )
    )

(println "testing if 3 < 4...")
(if (< 3 4)
    (println "three *is* less than 4!")
    (println "oops!")
    )

(println "testing if 3 < 4 (no else)...")
(if (< 3 4)
    (println "three *is* less than 4!")
    )

(println "testing if 3 < 2...")
(if (< 3 2)
    (println "oops!")
    (println "three *is not* less than 2!")
    )

(println "testing if 3 < 2 (no else)...")
(if (< 3 2)
    (println "oops!")
    )

(println "done")
