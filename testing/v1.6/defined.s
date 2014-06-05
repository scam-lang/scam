(include "reflection.lib")

(define methodList (array (symbol "-") '+ (symbol "+!+")))
(inspect methodList)

(define i 0)

(while (< i (length methodList))
    (define item (getElement methodList i))
    (if (defined? (__id item) this)
        (println item " is defined!")
        (println item " is not defined.")
        )
    (set! i (+ i 1))
    )
(inspect (defined? 'defined? this))
