function foreach($v,items,$body)
    {
    while (items != :null)
        {
	$v = head(items);
	force($body);
	items = tail(items);
	}
    }

var i;
var a = array(1,2,3);

foreach(i,array(1,2,3,4))
    {
    println("i is ", i);
    }
    
foreach(a[2],array(1,2,3,4))
    {
    println("a[2] is ", a[2]);
    }
    
