(include "reflection.lib")

(define (run1) 
    (inspect (+ (polar 3 1.423) (polar 7 2.453)))
    (inspect (+ (rectangular 3 1.423) (rectangular 7 2.453)))
    (inspect (+ (polar 3 1.423) 0))
    (inspect (+ (rectangular 3 1.423) 0))
    )

(define (polar mag ang)
    (list 'polar mag ang)
    )

(define (rectangular x y)
    (list 'rectangular x y)
    )

(define (polar? p) (and (pair? p) (eq? (car p) 'polar)))
(define (rectangular? p) (and (pair? p) (eq? (car p) 'rectangular)))
(define (polar->rectangular p)
    (rectangular (* (cadr p) (cos (caddr p))) (* (cadr p) (sin (caddr p))))
    )
(define (rectangular->polar r)
    (define x (cadr r))
    (define y (caddr r))
    (polar (expt (+ (* x x) (* y y)) 0.5) (atan y x))
    )
(define (polar+polar a b)
    (rectangular->polar
        (rectangular+rectangular
            (polar->rectangular a)
            (polar->rectangular b)
            )
        )
    )
(define (polar+rectangular a b)
    (rectangular->polar
        (rectangular+rectangular
            (polar->rectangular a)
            (rectangular b 0)
            )
        )
    )
(define (polar+number a b)
    (rectangular->polar
        (rectangular+rectangular
            (polar->rectangular a)
            (rectangular b 0)
            )
        )
    )
(define (rectangular+rectangular a b)
    (rectangular (+ (cadr a) (cadr b)) (+ (caddr a) (caddr b)))
    )
(define (rectangular+number a b)
    (rectangular+rectangular a (rectangular b 0))
    )
(redefine (+ a b)
    (cond
        ((polar? a)
            (cond
                ((polar? b) (polar+polar a b))
                ((rectangular? b) (polar+rectangular a b))
                (else (polar+number a b))
                )
            )
        ((rectangular? a)
            (cond
                ((polar? b) (polar+rectangular b a))
                ((rectangular? b) (rectangular+rectangular a b))
                (else (rectangular+number a b))
                )
            )
        (else
            (cond
                ((polar? b) (polar+number b a))
                ((rectangular? b) (rectangular+number b a))
                (else ((prior) a b))
                )
            )
        )
    )

(define (run2)
    (println "(add two three) is " (translate (add two three)))
    (println "(add zero five) is " (translate (add zero five)))
    (println "(add five zero) is " (translate (add five zero)))
    (println "(add nine nine) is " (translate (add nine nine)))
    )

(define (identity x) x)

(define (increment number)
    (define (next-higher-number incrementer)
        (define (resolve base)
            (incrementer ((number incrementer) base))
            )
        resolve
        )
    next-higher-number
    )

(define (zero incrementer)
    identity
    )

(define (one incrementer) 
    (define (resolver base)
        (incrementer base)
        )
    resolver
    )

(define (two incrementer)
    (lambda (base)
        (incrementer (incrementer base))
        )
    )

(define (three incrementer)
    (lambda (base)
        (incrementer (incrementer (incrementer base)))
        )
    )

(define (four incrementer)
    (lambda (base)
        (incrementer (incrementer (incrementer (incrementer base))))
        )
    )

(define (five incrementer)
    (lambda (base)
        (incrementer (incrementer (incrementer (incrementer (incrementer base)))))
        )
    )

(define (six incrementer)
    (lambda (base)
        (incrementer (incrementer (incrementer (incrementer (incrementer (incrementer base))))))
        )
    )

(define (seven incrementer)
    (lambda (base)
        (incrementer (incrementer (incrementer (incrementer (incrementer (incrementer (incrementer base)))))))
        )
    )

(define (eight incrementer)
    (lambda (base)
        (incrementer (incrementer (incrementer (incrementer (incrementer (incrementer (incrementer (incrementer base))))))))
        )
    )

(define (nine incrementer)
    (lambda (base)
        (incrementer (incrementer (incrementer (incrementer (incrementer (incrementer (incrementer (incrementer (incrementer base)))))))))
        )
    )

(define (inc x) (cons 'x x))
(define base ())

(define numbers
    (list
        (list ((zero inc) base) 'zero)
        (list ((one inc) base)  'one)
        (list ((two inc) base)  'two)
        (list ((three inc) base)  'three)
        (list ((four inc) base)  'four)
        (list ((five inc) base)  'five)
        (list ((six inc) base)  'six)
        (list ((seven inc) base)  'seven)
        (list ((eight inc) base)  'eight)
        (list ((nine inc) base) 'nine)
        )
    )

(define (translate number)
    (define result (assoc ((number inc) base) numbers))
    (if (eq? result #f)
        ((number inc) base)
        (cadr result)
        )
    )

(define (add a b)
    (lambda (incrementer)
        (lambda (base) ((a incrementer) ((b incrementer) base)))
        )
    )

(define (run3)
    (inspect (define->lambda '(define (plus a b) (+ a b))))
    )

(define 
(define (run4)
    (inspect ((extractor 0 1) '((1 2) 3)))
    )

(define (extractor @)
    (define (iter pointers)
        (cond
            ((equal? pointers '(0)) car)
            ((equal? pointers '(1)) cdr)
            ((eq? (car pointers) 0)
                (lambda (x) ((iter (cdr pointers)) (car x))))
            (else
                (lambda (x) ((iter (cdr pointers)) (cdr x))))
            )
        )
    (iter @)
    )
;(run1)
;(run2)
(run3)
(run4)
