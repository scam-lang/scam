(define key 0)
(define left 1)
(define right 2)

(define (printTree t)
    (if (null? t)
        (print "()")
        (begin
            (print "(" (getElement t key) " ")
            (printTree (getElement t left))
            (print " ")
            (printTree (getElement t right))
            (print ")")
            )
        )
    )

(define insert
    (lambda (t v)
        (cond
            ((null? t)
                (set!  t  (array nil nil nil))
                (setElement t key v)
                )
            ((< v (getElement t key))
                (setElement t left (insert (getElement t left) v))
                )
            (else
                (setElement t right (insert (getElement t right) v))
                )
            )
        t
        )
    )

(define find 
    (lambda (t v)
        (cond
            ((or (null? t) (eq? v (getElement t key)))
                t
                )
            ((< v (getElement t key))
                (find (getElement t left) v)
                )
            (else
                (find (getElement t right) v)
                )
            )
        )
    )

(define (lookup t i)
    (define result (find t i))
    (cond
        ((null? result)
            "not found!"
            )
        ((and (null? (getElement result left)) (null? (getElement result right)))
            "it's a leaf!"
            )
        (else
            "it's an interior node!"
            )
        )
    )

(define (main)
    (define i)
    (define num nil)
    (define input (array 1  2  3))
    (define search (array 4  2  3))
    (define t nil)

    (set! i 0)
    (while (< i (length input))
        (set! num (getElement input i))
        (set! t (insert t num))
        (print num " inserted.\n")
        (++ i)
        )
    
    (print "insertion phase complete, tree is ")
    (printTree t)
    (print ".\n")

    (set! i 0)
    (while (< i (length search))
        (set! num (getElement search i))
        (print "looking for "  num  ": "  (lookup t num) "\n")
        (++ i)
        )
    (print "good-bye!\n")
    )

(print "hello\n")
(main)
