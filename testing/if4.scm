(include "pretty.lib")

(define if
    (lambda (# test $tBranch $)
        (define oldIf (get 'if (get '__context __context)))
        (pp oldIf)
        (println "the test is "  test)
        (pass oldIf # test $tBranch $)
        )
    )

(pp if)

(println "testing if 3 is less than 4...")
(if (< 3 4)
    (println "three *is* less than 4!")
    (println "oops!")
    )
