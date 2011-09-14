(include "clone.lib")

(define (f) (println "starting first") (println "first"))
(define p f)
(define (f) (println "starting second") ((prior)) (println "second"))
(setPrior f p)
(define p f)
(define (f) (println "starting third") ((prior)) (println "third"))
(setPrior f p)
(define p f)
(define (f) (println "starting fourth") ((prior)) (println "fourth"))
(setPrior f p)


(f)

(define (g) (println "starting first") (println "first"))
(define (g) (println "starting second") ((prior)) (println "second"))
(define (g) (println "starting third") ((prior)) (println "third"))
(define (g) (println "starting fourth") ((prior)) (println "fourth"))

(g)
