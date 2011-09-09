(include "assign1.scm")

(define (inspect # $item)
    (print $item " is " (eval $item #) " ")
    )

(println "problem 1...\n")
(if (defined? 'run1 this)
    (println "no testing for problem 1\n")
    (println "problem 1 not implemented\n")
    )

(println "problem 2...\n")
(if (defined? 'run2 this)
    (begin
        (inspect (zeno_cost 20 150 0.25))
        (println "[it should be 199.951172]\n")
        )
    (println "problem 2 not implemented\n")
    )

(println "problem 3...\n")
(if (defined? 'run3 this)
    (begin
        (inspect (min8 0 0 0 0 0 0 0 0))
        (println "[it should be 0]\n")
        )
    (println "problem 3 not implemented\n")
    )

(println "problem 4...\n")
(if (defined? 'run4 this)
    (begin
        (inspect (root3 27.0))
        (println "[it should be 3.0]\n")
        )
    (println "problem 4 not implemented\n")
    )

(println "problem 5...\n")
(if (defined? 'run5 this)
    (begin
        (pt 0)
        (println "[it should print out 1]\n")
        )
    (println "problem 5 not implemented\n")
    )

(println "problem 6...\n")
(if (defined? 'run6 this)
    (begin
        (inspect (zorp 1 (lambda (x) (* x x))))
        (println "[it should be 1]\n")
        )
    (println "problem 6 not implemented\n")
    )

(println "problem 7...\n")
(if (defined? 'run7 this)
    (begin
        (inspect (square 1))
        (println "[it should be 1]")
        (inspect (halve 2))
        (println "[it should be 1]")
        (inspect (babyl* 1 1))
        (println "[it should be 1]\n")
        )
    (println "problem 7 not implemented\n")
    )

(println "problem 8...\n")
(if (defined? 'run8 this)
    (begin
        (inspect (ecf 0))
        (println "[it should be 2.0]\n")
        )
    (println "problem 8 not implemented\n")
    )

(println "problem 9...\n")
(if (defined? 'run9 this)
    (begin
        (inspect (ramanujan 0))
        (println "[it should be 1.0]\n")
        )
    (println "problem 9 not implemented\n")
    )

(println "problem 10...\n")
(if (defined? 'run10 this)
    (begin
        (inspect (ramanujan2 1 2))
        (println "[it should be 1.732051]\n")
        )
    (println "problem 10 not implemented]\n")
    )
