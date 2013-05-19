include("basics");
include("matrix");

var linecheck =
    {
    var mode = :dynamic;
    var mat = allocate(10);
    var location = ".sway_linecheck";

    function bundle(fileName,lineNumber) { this; }

    function start()
        {
        var read = catch(open(location,:read));
        if (not(read is :ERROR))
            {
            var s;
            var ic = 0;
            var len = length(mat);
            var fn,ln;
            var oldPort = setPort(read);

            fn = readToken();
            while(eof?() != :true)
                {
                if (len == ic) { expand(); }
                ln = readInt();
                mat[ic] = bundle(fn,ln);
                ic = ic + 1;
                fn = readToken();
                }

            close(read);
            setPort(oldPort);
            }
        }
    function expand()
        {
        var i = 0;
        var len = length(mat);
        var mat2 = allocate(2 * length(mat));
        while (i < len)
            {
            mat2[i] = mat[i];
            i = i + 1;
            }
        mat = mat2;
        }
    function logger($current,$rest)
        {
        if ($current . code != :null)
            {
            var fn = file($current . code);
            var ln = line($current . code);
            logInfo(fn,ln);
            force($current);
            }
        }
    function logInfo(fn,ln)
        {
        var i,found;
        //search the matrix and log if missing

        if(mat[length(mat) - 1] != :null) //expansion if necessary
            {
            expand();
            }

        //loop to check if current file/line is already in the matrix

        i = 0;
        found = :false;
        while(not(found) && mat[i] != :null)
            {
            if (mat[i] . lineNumber == ln && mat[i] . fileName == fn)
                {
                found = :true;
                }
            else
                {
                i = i + 1;
                }
            }

        //if empty slot, then must be missing, so add it
        if (mat[i] == :null)
            {
            println("logging file ",fn,", line ",ln);
            mat[i] = bundle(fn,ln);
            }
        }
    function finish() //write out the matrix to file
        {
        var j;
        var write = open(location,:write);
        var oldPort = setPort(write);

        j = 0;
        while (j < length(mat))
            {
            if (mat[j] != :null)
                {
                println(mat[j] . fileName,"\t",mat[j] . lineNumber);
                }
            j = j + 1;
            }	
        
        close(write);
        setPort(oldPort);
        }
    function decompose(current)
        {
        //println("current is ",current);
        if (current == :null)
            {
            return :done;
            }
        else if (type(current) == :STATEMENT)
            {
            logInfo(file(current),line(current));
            //now look at the expression
            decompose(head(current));
            }
        else if (current is :BLOCK || current is :SIMPLE_BLOCK)
            {
            decompose(head(current));
            }
        else if (current is :JOIN || current is :LIST
              || current is :CALL || current is :XCALL || current is :BINARY)
            {
            decompose(head(current));
            decompose(tail(current));
            }
        }
    start();
    this;
    };

function __clean_up__()
    {
    println("cleaning up...");
    linecheck . finish();
    }

function function($name,$params,$body)
    {
    var func = shadowed(:function);
    var f = func($name,$params,$body);

    if (linecheck . mode == :static)
        {
        //println("static decomposition...");
        linecheck . decompose($body . code);
        }
    else if (linecheck . mode == :dynamic)
        {
        //println("dynamic logging...");
        f . filter = linecheck . logger;
        }
    f;
    }

var dummy = linecheck . mode = :static;

function f(x)
    {
    function go()
        {
        if (x % 2 == 0)
            {
            println("even");
            }
        else
            {
            println("odd");
            }
        }
    go();
    }

linecheck . mode = :none;

println("done");
f(0);
