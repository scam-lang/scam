include("sway.s");
function bundle(x,y) { this; }

var b = bundle(2,3);

inspect(b . x);
inspect(b . y);

b . x = :hello;
b . y = :world;

inspect(b . x);
inspect(b . y);
