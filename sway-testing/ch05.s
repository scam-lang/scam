/* from ch04 */

function term(a,n)
   {
   function value(x)
       {
       a * (x ^ n);
       }
    this;
    }

/* new stuff */

var c = term(7,0);
   
inspect(c . value(10));
inspect(c . value(1000));
