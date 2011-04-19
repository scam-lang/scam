function cons($)
    {
    return $;
    }

function car(x)
    {
    //force is a primitive for evaluating
    //a delayed object
    return force(x . 0);
    }

function cdr(x)
    {
    //print("the cdr is ", x . 1, "\n");
    return force(x . 1);
    }

function numbersStartingFrom(start)
    {
    //print("start is ", start, "\n");
    return cons(start, numbersStartingFrom(start + 1));
    }

function displayStream(str, count)
    {
    print("[ ");
    while (count > 0)
        {
        print(car(str), " ");
        str = cdr(str);
        count = count - 1;
        }
    print("]");
    }

displayStream(numbersStartingFrom(2),100);
print("\n");
