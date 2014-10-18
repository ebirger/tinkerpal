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

meminfo();
