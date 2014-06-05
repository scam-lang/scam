(include "pretty.lib")

(define (f x)
    (define (g)
        (return (return 5))
        )
     (g)
     (+ x 1)
     )

(pretty f)

(println "result should be 5...")
(inspect (f 5))
