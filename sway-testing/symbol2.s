function makeThunk($x) { $x; }

function sqr(x) { x * x; }

var x = 10;
var params;

var thunk;


println("created a thunk...");
thunk = makeThunk(x);
println("thunk is ", thunk);
println("changing a thunk...");
inspect(sqr);
inspect(thunk . code = head(sqr . parameters));
ppObject(thunk);
println("forcing thunk, x is ID, x is declared globally with value ",x,"...");
inspect(force(thunk));
println("changing a thunk...");
inspect(thunk . code = symbol("x"));
ppObject(thunk);
println("forcing thunk, x is SYMBOL...");
inspect(force(thunk));
