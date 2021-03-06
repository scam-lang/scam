function cons($a, $b)
    {
    return array($a,$b);
    }

function car(x)
    {
    //force is a primitive for evaluating
    //a delayed object
    return force(x . 0);
    }

function cdr(x)
    {
    return force(x . 1);
    }

function numbersStartingFrom(start)
    {
    return cons(start, numbersStartingFrom(start + 1));
    }

function sieve(primes)
    {
    function predicate(x)
        {
        return (x % car(primes)) == 0;
        }

    return cons(car(primes),sieve(remove(predicate,cdr(primes))));
    }

function remove(pred, items)
    {
    if (pred(car(items)))
        {
        return pred remove cdr(items);
        }
    else
        {
        return car(items) cons (pred remove cdr(items));
        }
    }

function displayStream(str, count)
    {
    print("[ ");
    while (count > 0)
        {
        //if (car(str) == 211) { defines(); }
        print(car(str), " ");
        str = cdr(str);
        count = count - 1;
        }
    print("]");
    }


var primes = sieve(numbersStartingFrom(2));

println("this takes a while...");
displayStream(primes,50);
print("\n");
