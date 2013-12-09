function test(j, b)
{
    console.log(111);
    return 1;
}

function num(q)
{
    console.log(q);
}

function factorial(n) {
    console.log("factorial : " + n + " " + (n - 1));
    if (n == 0)
        return 1;

    return n * factorial(n - 1);
}

function main()
{
    var x, y, z;
    console.log("====1");
    x = test;
    console.log("====2");
    num(5);
    console.log("====3");
    x();
    console.log("====4");
    test();
    console.log("====5");

    console.log(factorial(5));
    debug.assert(factorial(5), 120);
    console.log("====6");
    y = function() { console.log(222); return 3; };
    console.log("====7");
    console.log(y());
    console.log("====8");
    z = y;
    console.log("====9");
    z();
    console.log("====10");
    debug.assert(x == test, true);
    console.log("====11");

    debug.assert(x(), 1);
    console.log("====12");
    debug.assert(test(), 1);
    var ff = function(x, y) { return x + y; };
    console.log(ff.call(null, 1, 2));
    debug.assert(ff.call(null, 1, 2), 3);
}

var kuku = function pupu() {
    console.log("kuku");
    debug.assert_cond(kuku === pupu);
    return 1;
};

debug.assert(kuku(), 1);
debug.assert(pupu, undefined);
main();
