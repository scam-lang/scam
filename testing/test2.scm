(include "assign2.scm")

(println "problem 1...\n")
(if (defined? 'run1 this)
    (begin
        (inspect (+ 2 3))
        (println "    [it should be 5]")
        (inspect (+ (polar 1 .707) (polar 1 .707)))
        (println "    [it should be magnitude 2 and angle .707]")
        )
    (println "problem 1 not implemented\n")
    )

(println "problem 2...\n")
(if (defined? 'run2 this)
    (begin
        (inspect add)
        (println)
        (inspect numbers)
        (println)
        (inspect (translate (add two three)))
        (println "    [it should be five]\n")
        )
    (println "problem 2 not implemented\n")
    )
    

(println "problem 3...\n")
(if (defined? 'run3 this)
    (begin
        (inspect (define->lambda '(define (plus a b) (+ a b))))
        (println "    [it should be (define plus (lambda (a b) (+ a b)))]\n")
        )
    (println "problem 3 not implemented\n")
    )

(println "problem 4...\n")
(if (defined? 'run4 this)
    (begin
        (inspect ((extractor 1 1 0) '(1 2 3 4 5)))
        (println "    [it should be 3]")
        )
    (println "problem 4 not implemented\n")
    )

(println "problem 5...\n")
(if (defined? 'run5 this)
    (begin
        (inspect (matrix-*-matrix '((1 2) (3 4))'((1 0) (0 1))))
        (println "    [it should be ((1 2) (3 4))]\n")
        )
    (println "problem 5 not implemented\n")
    )

(println "problem 6...\n")
(if (defined? 'run6 this)
    (begin
        (inspect (infix->postfix '(2 + 3 * x ^ 5 + a)))
        (println "    [it should be (2 3 x 5 ^ * a + +) or equivalent]\n")
        )
    (println "problem 6 not implemented\n")
    )

(println "problem 7...\n")
(if (defined? 'run7 this)
    (begin
        (inspect (powerSet '(a b)))
        (println "    [it should be (() (a) (b) (a b))]")
        )
    (println "problem 7 not implemented\n")
    )

(println "problem 8...\n")
(if (defined? 'run8 this)
    (begin
        (inspect
            (depthStat (treeNode 1 (treeNode 1 nil nil) (treeNode 1 nil nil)))
            )
        (println "    [it should be 1.0]\n")
        )
    (println "problem 8 not implemented\n")
    )

(println "problem 9...\n")
(if (defined? 'run9 this)
    (begin
        (inspect (+ 2 3))
        (println "    [it should be 5]\n")
        (inspect ((dot (+ (Polar 1 .707) (Polar 1 .707)) toString)))
        (println "    [it should be magnitude 2 and angle .707]\n")
        (inspect ((dot (+ (Rectangular 1 1) (Integer 1)) toString)))
        (println "    [it should be real 2 and imaginary 1]\n")
        (inspect ((dot (+ (Real 2.3) (Integer 1)) value)))
        (println "    [it should be real 3.3]\n")
        )
    (println "problem 9 not implemented\n")
    )

(println "problem 10...\n")
(if (defined? 'run10 this)
    (begin
        (inspect ((dot (coerce (Integer 5) 'Real) value)))
        (println "[it should be real 5.0]\n")
        )
    (println "problem 10 not implemented]\n")
    )
