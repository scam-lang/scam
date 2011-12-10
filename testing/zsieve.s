(define (cons # $a $b)
    (array # $a $b)
    )

(define (car x)
    (eval (getElement x 1) (getElement x 0))
    )

(define (cdr x)
    (eval (getElement x 2) (getElement x 0))
    )

(define (numbersStartingFrom start)
    (cons start (numbersStartingFrom (+ start 1)))
    )

(define (sieve primes)
    (define (predicate x)
        (== (% x (car primes)) 0)
        )
    (cons (car primes) (sieve (remove predicate (cdr primes))))
    )

(define (remove pred items)
    (if (pred (car items))
        (remove pred (cdr items))
        (cons (car items) (remove pred (cdr items)))
        )
    )

(define (displayStream str count)
    (print "[ ")
    (while (> count 0)
        (print (car str) " ")
        (set 'str (cdr str))
        (set 'count (- count 1))
        )
    (print "]")
    )


(define primes (sieve (numbersStartingFrom 2)))

(println "this takes a while...")
(displayStream primes 50)
(println)
