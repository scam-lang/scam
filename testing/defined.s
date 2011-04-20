(include "reflection.s")

(define methods (array (symbol "-") '+ (symbol "++")))
(inspect (length methods))
(inspect methods)

(define i 0)

(while (< i (length methods))
    (define item (getElement methods i))
    (if (defined? item this)
        (println item " is defined!")
        (println item " is not defined.")
        )
    (set! 'i (+ i 1))
    )
