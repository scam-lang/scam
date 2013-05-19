function foreach(#,$v,items,$body)
    {
    while (items != null)
        {
        set!($v,car(items),#);
        eval($body,#);
        items = cdr(items);
        }
    }

var i;
var a = array(1,2,3,4);

foreach(i,a)
    {
    println("i is ", i);
    }
