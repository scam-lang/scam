(include "reflection.lib")
;(compile-reflection)
(include "../../proglan/table.scm")
(include "pretty.lib")


(define (run1) 
    (inspect (+ (polar 1 .707) (polar 1 .707)))
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

(define (define->lambda code)
    (list
        'define
        (car (cadr code)) ;function name
        (cons 'lambda (cons (cdr (cadr code)) (cddr code)))
        )
    )
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

(define (run7)
    (inspect (powerSet '(a b c d)))
    )

(define (powerSet items)
    (define (interleave a b)
        (cond
            ((null? a) b)
            ((null? b) a)
            ((or (null? (car a)) (< (length (car a)) (length (car b))))
                (cons (car a) (interleave (cdr a) b)))
            (else
                (cons (car b) (interleave a (cdr b))))
            )
        )
    (cond
        ((null? items) (list nil))
        (else 
            (define rest (powerSet (cdr items)))
            (define added (map (lambda (x) (cons (car items) x)) rest))
            (interleave rest added)
            )
        )
    )

(define (run8)
    (define t (treeNode 2 (treeNode 1 nil (treeNode 4 nil nil)) (treeNode 0 nil nil)))
    (inspect t)
    (inspect (depthStat t))
    )

(define (newTree) ())

(define (treeNode value left right)
    (list 'binaryTree value left right)
    )

(define (extract-leaves t)
    (define (iter root depth)
        (cond
            ((and (null? (caddr root)) (null? (cadddr root)))
                (list (list (cadr root) depth)))
            ((null? (caddr root))
                (iter (cadddr root) (+ depth 1)))
            ((null? (cadddr root))
                (iter (caddr root) (+ depth 1)))
            (else
                (append
                    (iter (cadddr root) (+ depth 1))
                    (iter (caddr root) (+ depth 1))
                    )
                )
            )
        )
    (iter t 0)
    )

(define (accumulate op base items)
    (cond
        ((null? items) base)
        (else (op (car items) (accumulate op base (cdr items))))
        )
    )
(define (depthStat t)
    (define depths (map cadr (extract-leaves t)))
    (define sum (accumulate + 0 depths))
    (define count (accumulate (lambda (x y) (+ 1 y)) 0 depths))
    (inspect depths)
    (inspect sum)
    (inspect count)
    (/ sum (real count))
    )

(define (run9)
    (inspect (((((Polar 3 1.423) 'add) (Polar 7 2.453)) 'toString)))
    ;(inspect (+ (rectangular 3 1.423) (rectangular 7 2.453)))
    ;(inspect (+ (polar 3 1.423) 0))
    ;(inspect (+ (rectangular 3 1.423) 0))
    )


(define (Integer i)
    (define (add x)
        (cond
            ((is? x 'Integer) (Integer (+ i ((x 'value)))))
            (else (((promote) 'add) x))
            )
        )
    (define (promote) (Real (real i)))
    (define (demote) __context)
    (define (value) i)
    (define (toString) (string i))
    this
    )

(define (Real r)
    (define (add x)
        (cond
            ((is? x 'Integer) (Real (+ r ((x 'value)))))
            ((is? x 'Real) (Real (+ r ((x 'value)))))
            (else (((promote) 'add) x))
            )
        )
    (define (promote) (Rectangular r 0))
    (define (demote) (Integer (integer r)))
    (define (value) r)
    (define (toString) (string r))
    this
    )

(define (Rectangular x y) 
    (define (add a)
        (cond
            ((is? a 'Integer) (Rectangular (+ x ((a 'value))) y))
            ((is? a 'Real) (Rectangular (+ x ((a 'value))) y))
            ((is? a 'Polar) (add ((a 'convert))))
            (else
                (Rectangular
                    (+ x (car ((a 'value))))
                    (+ y (cadr ((a 'value))))
                    )
                )
            )
        )
    (define (convert)
        (apply Polar (cdr (rectangular->polar (rectangular x y))))
        )
    (define (promote) __context)
    (define (demote) (Real (real x)))
    (define (value) (list x y))
    (define (toString) (string (list 'rectangular x y)))
    this
    )
         
(define (Polar m a) 
    (define (add a)
        (cond
            ((is? a 'Integer) (((((convert) 'add) a) 'convert)))
            ((is? a 'Real) (((((convert) 'add) a) 'convert)))
            ((is? a 'Rectangular) ((((a 'add) __context) 'convert)))
            (else (((((convert) 'add) a) 'convert)))
            )
        )
    (define (convert)
        (apply Rectangular (cdr (polar->rectangular (polar m a))))
        )
    (define (promote) __context)
    (define (demote) (Real (car (((convert) 'value)))))
    (define (value) (list m a))
    (define (toString) (string (list 'polar m a)))
    this
    )
(redefine (+ a b)
    (cond
        ((object? a) ((a 'add) b))
        (else ((prior) a b))
        )
    )

(run1)
(run2)
(run3)
(run4)
(run7)
(run8)
(run9)
