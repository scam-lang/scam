include("sway.s");

function heap(items,op)
    {
    var size = length(items);

    function leftChild(x) { 2 * x + 1; }
    function rightChild(x) { 2 * x + 2; }
    function deleteMin()
        {
        var temp = items[0];
        items[0] = items[size - 1];
        items[size - 1] = temp;
        size = size - 1;
        heapify(0);
        temp;
        }
    function heapify(root)
        {
        var extreme;
        var newRoot;

        if (leaf?(root)) { return :ok; }
        
        extreme = findExtremalChild(root);

        if (extreme == items[root]) { return :ok; }

        if (extreme == items[leftChild(root)])
            {
            newRoot = leftChild(root);
            }
        else
            {
            newRoot = rightChild(root);
            }
        items[newRoot] = items[root];
        items[root] = extreme;
        heapify(newRoot);
        }
    function findExtremalChild(root)
        {
        var extreme;

        extreme = extremal(op,items[root],items[leftChild(root)]);

        if (rightChild(root) >= size)
            {
            extreme;
            }
        else
            {
            extremal(op,extreme,items[rightChild(root)]);
            }
        }

    function build-heap()
        {
        var i;

        for (i = size - 1, i >= 0, i = i - 1)
            {
            println("heapifying element ",i);
            catch(heapify(i));
            println("element ",i," has been heapified");
            }
        }

    function leaf?(x)
        {
        leftChild(x) >= size;
        }

    println("about to build-heap...");
    build-heap();
    this;
    }

function heap-sort(items,op)
    {
    var i;
    var h;

    h = heap(items,op);

    while (h . size > 0)
        {
        print(h . deleteMin());
        if (h . size > 0,print(" "));
        }
    }

function extremal(op,a,b)
    {
    if (a op b,a,b);
    }

var a = array(3,6,3,9,4,10,5,29,4,6,0,20,25,16,88,0,31);

heap-sort(a,>);
println("\n");

inspect(a);
inspect(extremal . parameters);
