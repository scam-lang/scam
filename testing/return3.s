(define i 3)
(define j 8)

(define (even? x) (= (% x 2) 0))

(define (f)
    (define i 3)
    (define j 2)
    (define x (* i j))

    (define r (catch (return (* i j))))
    ;(define r (catch (return x)))

    (println "r is " (r 'code))
    (for (set! i 0) (< i 5) (++ i)
        (for (set! j 1) (< j 5) (++ j)
            (println "i is " i " and j is " j)
            (if (and (even? j) (= j i))
                (begin
                    (println "trying to leave f...")
                    (println "return result should be current value of i and j")
                    (throw r)
                    )
                )
            )
        )
    )

(define (g)
    (define z)
    (println "g executing...")
    (set! z (f))
    (println "g's z is " z)
    (println "g done.")
    z
    )

(println (g))
(println "done")
