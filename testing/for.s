var i = 0;

function for(init, $test, $increment, $body)
    {
    while(force($test))
        {
	force($body);
	force($increment);
	}
    }

function for2(init, $test, $increment, $body)
    {
    if(force($test))
        {
	force($body);
	force($increment);
	for2(init, $test, $increment, $body);
	}
    }

for(i = 1, i < 10, i = i + 2)
    {
    print("the value of i is ");
    print(i);
    print("\n");
    }
for2(i = 1, i < 10, i = i + 2)
    {
    print("the value of i is ");
    print(i);
    print("\n");
    }
