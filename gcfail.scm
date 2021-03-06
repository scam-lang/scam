(define (root3 number)
	(define (bt_root3 left right cubed)
		(define (avg a b)
			( * ( + a b ) 0.5 )
		)
		(define (cubed_avg left right)
			(expt ( avg left right ) 3 )
		)
		(define (abs_diff left right cubed)
			(define (abs a)
				(if (< a 0)
					(* -1 a)
					a
				)
			)
			(abs ( - (cubed_avg left right) cubed ) )
		)
		; If the midpoint^3 is left of cubed we set left to be left / 2
		; Otherwise if midpoint^3 is right of cubed we set right to be right / 2
		; Otherwise return
		(if ( >  (abs_diff left right cubed ) 1E-10 )
			(if ( < (cubed_avg left right) cubed )
				(bt_root3 (avg left right) right cubed)
				(bt_root3 left (avg left right) cubed)
			) 
		 	(avg left right)
		)
	)
	(bt_root3 0 number number)
)

(root3 0.00002)
