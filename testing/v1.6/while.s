(define (until # $test $)
    (println "forcing test")
    (while (not (eval $test #))
        (evalList $ #)
        )
    )
(define i 0)
(println "until as variadic function")
(until (= i 10)
    (print "i is " i "\n")
    (++ i)
    )
(println "user while that returns from global scope")
(until #f
    (println "returning!")
    (return 0)
    )

(print "you should not be seeing this message\n")
