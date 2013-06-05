var i = 0;

function for(init, #, $test, $increment, $body)
    {
    while(eval($test,#))
        {
        eval($body,#);
        eval($increment,#);
        }
    }

for(i = 1, i < 10, i = i + 2)
    {
    print("the increasing value of i is ");
    print(i);
    print("\n");
    }


for(i = 10, i > 0, i = i - 1, 
    {
    print("the decreasing value of i is ");
    print(i);
    print("\n");
    });

