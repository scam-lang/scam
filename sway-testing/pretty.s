function f(x)
    {
    for (i = 0, i < n, i += 1)
        {
        print(x);
        for-all(v,items)
            {
            print(v);
            print(items);
            }
        for-all(v,items,print(v),print(items));
        print(y);
        }
    for(i = 0,i < n, i += 1,print(x),print(y));
    while (i < n)
        {
        print(x);
        print(y);
        }
    while(i < n, print(x), print(y));
    }

pretty(f);
