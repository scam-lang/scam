(include "inherit.lib")

(define (x)
    (define parent nil)
    (define a 3)
    this
    )

(define b (new (x)))

(if (object? b)
    (println "b is an object!")
    (println "b is not an object!")
    )


(println "b . a is " (. b  a) ", (should be 3)")
(println "b . symbol(\"a\") is " ((. b symbol) "a") ", (should be a)");
(println "b . (symbol(\"a\")) is " (. b (symbol "a")) ", (should be 3)");
(println "b . 'a is " (. b 'a) " (should be 3)");

(assign (. b a) (+ (. b a) 1))
(println "b . a is " (. b a) " (should be 4)");
