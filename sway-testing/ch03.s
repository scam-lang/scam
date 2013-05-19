    function h(w)
        {
        w / 2.0;
        }

    function g(w)
        {
        w * w;
        }

    function ratio(w,dw,h)
        {
        var dh = h(w + dw) - h(w);
        dh / dw;
        }

inspect(ratio(40,0.01,h));
inspect(ratio(40,0.0001,h));
inspect(ratio(20,0.0001,h));

inspect(ratio(100,1,g));
inspect(ratio(100,0.1,g));
inspect(ratio(100,0.01,g));
inspect(ratio(100,0.001,g));
