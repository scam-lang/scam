(define (f)
    (define x)
    (println "entering f...")
    (if #t
        (println "entering block")
        (return 3)
        (println "leaving block (this should not be displayed)")
        5
        )
    (print "leaving f (this should not be displayed)\n")
    ;(return 22)
    (print "really leaving f (this should not be displayed)\n")
    10
    )

(define z 0)
(print "f() should return 3\n") 
(set! z (f))
(print "f() returns " z "\n") 
(println "done");
