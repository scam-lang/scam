(include "pretty.lib")
(randomSeed (int (time)))

(define (author)
    (println "John C. Lusth")
    )

(define (range b e s)
    (if (>= b e)
        nil
        (cons b (range (+ b s) e s))
        )
    )

(define (for-loop seq f)
    (cond
        ((null? seq) nil)
        (else (f (car seq)) (for-loop (cdr seq) f))
        )
    )

(define (run1)
    (for-loop (range 0 10 1) (lambda (i) (inspect i)))
    )

(define (pval f @)
    (define first-k @)
    (lambda (@) (apply f (append first-k @)))
    )

(define (run2)
    (define (f x y z) (+ x y z))
    (inspect ((pval f 1 2) 3))
    )

(define (infix->prefix expr)
    (define (rewrite op expr mode)
        (cond
            ((< (length expr) 3) expr)
            ((and (eq? (cadr expr) op) (eq? mode 'left))
                (cons
                    (list (cadr expr) (car expr) (caddr expr))
                    (rewrite op (cdddr expr) mode)
                    )
                )
            ((eq? (cadr expr) op) ; right associative
                (define result (rewrite op (cddr expr) mode))
                (cons
                    (list (cadr expr) (car expr) (car result))
                    (cdr result)
                    )
                )
            (else
                (append
                    (list (car expr) (cadr expr))
                    (rewrite op (cddr expr) mode)
                    )
                )
            )
        )
    (define (iter ops expr)
        ;(inspect expr)
        ;(pause)
        (cond
            ((null? ops) expr)
            (else 
                (define result (rewrite (car ops) expr
                    (if (eq? (car ops) '^) 'left 'right)))
                (if (equal? result expr)
                    (iter (cdr ops) expr)
                    (iter ops result)
                    )
                )
            )
        )
    (car (iter '(^ / * - +) expr))
    )

(define (run3)
    (inspect (infix->prefix '(a)))
    (inspect (infix->prefix '(a + b)))
    (inspect (infix->prefix '(a + b * c + d)))
    (inspect (infix->prefix '(a - b - c - d)))
    )

(define (let2locals expr)
    (define (make-defines vars)
        (cond
            ((null? vars) nil)
            (else
                (cons
                    (list 'define (caar vars) (cadr (car vars)))
                    (make-defines (cdr vars))
                    )
                )
            )
        )
    (define (replace-let expr)
        (append
            (make-defines (cadr expr))
            (cddr expr)
            )
        )
    (define body (caddr expr))
    (inspect body)
    (if (and (pair? body) (eq? (car body) 'let))
        (append
            (list (car expr) (cadr expr))
            (replace-let body)
            )
        expr
        )
    )

(define (run4)
    (inspect (let2locals '(define (f) 1)))
    (inspect (let2locals '(define (f) (let ((a 1)) a))))
    (inspect (let2locals '(define (f) (let ((a 1)(b 2)) (+ a b) (+ a a)))))
    )

(define base 0)
(define (inc x) (+ 1 x))

(define two
    (lambda (i)
        (lambda (b)
            (i (i b))
            )
        )
    )

(define three
    (lambda (i)
        (lambda (b)
            (i (i (i b)))
            )
        )
 )

(define four
    (lambda (i)
        (lambda (b)
            (i (i (i (i b))))
            )
        )
    )

(define (cmult x y)
    (lambda (i)
        (lambda (b)
            ((x (y i)) b)
            )
        )
    )

(define (cmult x y)
    (lambda (i)
        (lambda (b)
            ((x (y i)) b)
            )
        )
    )

(define (cpow x y)
    (lambda (i)
        (lambda (b)
            (((y x) i) b)
            )
        )
    )

(define (run5)
    (inspect ((two inc) base))
    (inspect ((three inc) base))
    (inspect (((cmult two three) inc) base))
    (inspect (((cmult four three) inc) base))
    (inspect (((cpow two three) inc) base))
    (inspect (((cpow three two) inc) base))
    (inspect (((cpow three four) inc) base))
    (inspect (((cpow two four) inc) base))
    )

(define (powerSet items)
    (define (merge a b)
        ;(inspect (list 'merge a b))
        (cond
            ((null? a) b)
            ((null? b) a)
            ((< (length (car a)) (length (car b)))
                ;(inspect (length (car a)))
                ;(inspect (length (car b)))
                ;(println "a wins.")
                ;(pause)
                (cons (car a) (merge (cdr a) b)))
            (else
                ;(inspect (length (car a)))
                ;(inspect (length (car b)))
                ;(println "b wins.")
                ;(pause)
                (cons (car b) (merge a (cdr b))))
            )
        )
    (cond
        ((null? items) '(()))
        (else
            (define rest (powerSet (cdr items)))
            (merge rest (map (lambda (x) (cons (car items) x)) rest))
            )
        )
    )

(define (run6)
    (inspect (powerSet '(a b c)))
    )

(define (accumulate op base items)
    (cond
        ((null? items) base)
        (else (op (car items) (accumulate op base (cdr items))))
        )
    )

(define (dot-product a b)
    (accumulate + 0 (map * a b))
    )

(define (matrix-*-vector m v)
    (map (lambda (r) (dot-product r v)) m)
    )

(define (transpose m)
    (apply map (cons (lambda (@) @) m))
    )

(define (matrix-*-matrix a b)
    (define bt (transpose b))
    (map (lambda (v) (matrix-*-vector bt v)) a)
    )

(define (run7)
    (inspect (dot-product '(1 2 3) '(4 5 6)))
    )

(define (extract guide items)
    (define (iter store src)
        ;(inspect store)
        ;(inspect src)
        ;(pause)
        (cond
            ((null? src) store)
            ((equal? (car src) "h") (iter (car store) (cdr src)))
            (else (iter (cdr store) (cdr src)))
            )
        )
    (iter items (string guide))
    )

(define (run8)
    (inspect (extract 'th '(1 2 3 4)))
    )

(define asciibase (- (ascii "a") 1))

(define (word2int w)
    (cond
        ((null? w) 0)
        (else (+ (- (ascii (car w)) asciibase) (word2int (cdr w))))
        )
    )

(define words nil)

(define (overloadMinus fileName)
    (define p (open fileName 'read))
    (define (iter x items count)
        (cond
            ((eof?) items)
            (else
                (inspect count)
                (iter (readToken) (cons x items) (+ count 1))
                )
            )
        )
    (define oldPort (setPort p))
    (set! words (iter (readToken) nil 0))
    (close p)
    (setPort oldPort)
    (inspect words)
    (inspect (length words))
    )

(define (run9)
    (inspect (word2int "love"))
    (overloadMinus "words")
    )

;(run1)
;(run2)
;(run3)
;(run4)
;(run5)
;(run6)
;(run7)
;(run8)
(run9)

