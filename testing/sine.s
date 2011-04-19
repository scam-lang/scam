include("basics");

var amplitude = 1;
var frequency = 1;
var shift = 0;

var width;
var low = -2.0;
var high = 10.0;
var step = .25;

var x;

function sineWave(a,f,s)
    {
    function (x)
        {
	a * sin(f * x + s);
	};
    }

var s = sineWave(amplitude,frequency,shift);

var port = open("sine.dat",:write);
setPort(port);

println("# x\ty");
for (x = low, x <= high, x += step)
    {
    println(x,"\t",s(x));
    }

close(port);

system("cat sine.dat");
