function show(@)
    {
    apply(print,@);
    }

function fatal(@)
    {
    apply(print,@);
    exit(3);
    }

show("goodbye, ", "cruel", " world");
print("\n");
fatal("goodbye, ", "cruel", " world");
