(define (fib n)
    (if (< n 2)
        n
        (+ (fib (- n 1)) (fib (- n 2)))
        )
    )

(define x 0)
(define result)
(define t (time))

(define x 26)

opt(fib . code,:<,<);
opt(fib . code,:-,-);
opt(fib . code,:+,+);
opt(fib . code,:if,if);
opt(fib . code,:fib,fib);

(define result (fib x))
(display("fib(");
(display(x);
(display(") is ");
(display(result);
(display("\n");
(display(time() - t);
(display(" seconds");
(display("\n");
