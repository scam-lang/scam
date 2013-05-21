include("sway.lib");

var spot = SwayEnv;

while (spot != null)
    {
    var s = head(spot);
    println(prefix(s,stringUntil(s,"=")));
    spot = tail(spot);
    }
