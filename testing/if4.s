var if = function (test, $tBranch, $fBranch)
    {
    var oldIf = context . context . if;
    print("the test is ", test, "\n");
    oldIf(test)
        {
        while (:false) { println("huh"); }
        force($tBranch);
        }
    else
        {
        force($fBranch);
        }
    };
pp(if);

if (3 < 4)
    {
    print("three *is* less than 4!\n");
    }
else
    {
    print("oops!\n");
    }
