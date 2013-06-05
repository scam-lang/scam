function bundle(x,y) { this; }

var b;

b = bundle(2,3);

inspect(b . x);
inspect(b . y);

b . x = 'hello;
b . y = 'world;

inspect(b . x);
inspect(b . y);

var c = bundle(b,b);

c . x . y = 'WORLD;

inspect(b . y);
inspect(c . x . y);
inspect(c . y . y);
