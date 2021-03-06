(include "random.lib")

(define count 1)
(define (go)
    (define a (allocate (randomRange 1 10000)))
    (define actions (array
        (lambda () (include "../assoc.s"))
        (lambda () (include "../apply.s"))
        (lambda () (include "../a.s"))
        (lambda () (include "../block.s"))
        (lambda () (include "../call.s"))
        (lambda () (include "../chain.s"))
        (lambda () (include "../common.s"))
        (lambda () (include "../common2.s"))
        (lambda () (include "../common3.s"))
        (lambda () (include "../common4.s"))
        (lambda () (include "../defined.s"))
        (lambda () (include "../delay.s"))
        (lambda () (include "../except.s"))
        (lambda () (include "../extend.s"))
        (lambda () (include "../extension.s"))
        (lambda () (include "../arrays.s"))
        ))
    ((getElement actions (randomRange 0 (length actions))))
    (inspect (getElement a (randomRange 0 (length a))))
    (go)
    )
(go)
    
