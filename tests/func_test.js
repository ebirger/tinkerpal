function test(j, b)
{
    debug.dump(111);
    return 1;
}

function num(q)
{
    debug.dump(q);
}

function factorial(n) {
    debug.dump("factorial : " + n + " " + (n - 1));
    if (n == 0)
        return 1;

    return n * factorial(n - 1);
}

function main()
{
    var x, y, z;
    debug.dump("====1");
    x = test;
    debug.dump("====2");
    num(5);
    debug.dump("====3");
    x();
    debug.dump("====4");
    test();
    debug.dump("====5");

    debug.dump(factorial(5));
    debug.assert(factorial(5), 120);
    debug.dump("====6");
    y = function() { debug.dump(222); return 3; };
    debug.dump("====7");
    debug.dump(y());
    debug.dump("====8");
    z = y;
    debug.dump("====9");
    z();
    debug.dump("====10");
    debug.assert(x == test, true);
    debug.dump("====11");

    debug.assert(x(), 1);
    debug.dump("====12");
    debug.assert(test(), 1);
    var ff = function(x, y) { return x + y; };
    debug.dump(ff.call(null, 1, 2));
    debug.assert(ff.call(null, 1, 2), 3);
}

var kuku = function pupu() {
    debug.dump("kuku");
    debug.assert_cond(kuku === pupu);
    return 1;
};

debug.assert(kuku(), 1);
debug.assert(pupu, undefined);
main();
