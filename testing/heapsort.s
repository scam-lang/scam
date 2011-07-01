(define (heap items op)
    (define size (length items))

    (define (leftChild x) (+ (* 2 x) 1))
    (define (rightChild x) (+ (* 2 x) 2))
    (define (deleteExtreme)
        (define temp (getElement items 0))
        (setElement! items 0 (getElement items (- size 1)))
        (setElement! items (- size 1) temp)
        (set! 'size (- size 1))
        (heapify 0)
        temp
        )

    (define (heapify root)
        (define extreme nil)
        (define newRoot nil)

        (if (leaf? root) (return 'ok))
        
        (set! 'extreme (findExtremalChild root))

        (if (== extreme (getElement items root)) (return 'ok))

        (if (== extreme (getElement items (leftChild root)))
            (set! 'newRoot (leftChild root))
            (set! 'newRoot (rightChild root))
            )

        (setElement! items newRoot (getElement items root))
        (setElement! items root extreme)
        (heapify newRoot)
        )

    (define (findExtremalChild root)
        (define extreme nil)
        (set! 'extreme 
            (extremal op
                (getElement items root)
                (getElement items (leftChild root))))

        (if (>= (rightChild root) size)
            extreme
            (extremal op extreme (getElement items (rightChild root)))
            )
        )

    (define (build-heap)
        (define i nil)
        (for (set! 'i (- size 1)) (>= i 0) (set! 'i (- i 1))
            (println "heapifying element " i)
            (heapify i)
            (println "element " i " has been heapified")
            )
        )

    (define (leaf? x) (>= (leftChild x) size))

    (println "about to build-heap...")
    (build-heap)
    this
    )

(define (heap-sort items op)
    (define i nil)
    (define h nil)

    (set! 'h (heap items op))

    (while (> (get 'size h) 0)
        (print ((get 'deleteExtreme h)))
        (if (> (get 'size h) 0) (print " "))
        )
    (println "\n")
    )

(define (extremal op a b)
    (if (op a b) a b)
    )

(define a (array 3 6 3 9 4 10 5 29 4 6 0 20 25 16 88 0 31))

(heap-sort a >)

(inspect a)
(inspect (get 'parameters extremal))
