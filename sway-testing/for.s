var i = 0;

function for(#,init,$test,$increment,$body)
    {
    while(eval($test,#))
        {
        eval($body,#);
        eval($increment,#);
        }
    }

function for2(#,init,$test,$increment,$body)
    {
    function helper(spot)
        {
        if(eval($test,spot))
            {
            eval($body,spot);
            eval($increment,spot);
            helper(spot);
            }
        }
    helper(#);
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
