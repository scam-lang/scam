function parent(phrase)
    {
    function a() { "well, " + phrase + b(); }
    function b() { "all!"; }
    this;
    }
function child(phrase)
    {
    function b() { "there!"; }  //override parent's b
    function c() { "hi," + b . prior(); }
    proxy(parent(phrase));
    }

var obj = child("hello ");

println(obj . a());
println(obj . c());
