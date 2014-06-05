(include "pretty.lib")

(define (f x)
    (define (g)
        (return (return 5))
        )
     (g)
     )

(pretty f)

(println "result should be 5...")
(inspect (f 5))
