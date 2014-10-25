if (false)
    debug.assert(1, 0);
else
    good = 1;

debug.assert(good, 1);

good = 0;

if (false)
    debug.assert(1, 0);
else if (1 == 0)
    debug.assert(1, 0);
else
    good = 1;

debug.assert(good, 1);

good = 0;

if (false)
    debug.assert(1, 0);
else if (1 == 1)
    good = 1;
else
    debug.assert(1, 0);

debug.assert(good, 1);

good = 0;

if (true)
    good = 1;
else if (1 == 1)
    debug.assert(1, 0);
else
    debug.assert(1, 0);

debug.assert(good, 1);

/* Run various API functions */
console.log('--------------');
meminfo();
console.log('--------------');
describe(describe);
describe(3);
console.log('--------------');
debug.dump_env();
console.log('--------------');
debug.assert_exception(function() { compile(function() { }); });
console.log(-1.1);

debug.assert_exception(function() { var x = 3 5; });
