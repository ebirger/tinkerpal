var i = 1, val;

switch (i)
{
}

switch (i)
{
case 1:
    val = 1;
case 2:
    val = 2;
    break;
case 3:
    debug.assert(3 == 0);
    break;
case 4:
    debug.assert(4 == 0);
    break;
}

debug.assert(val, 2);

var b = i;

switch (i)
{
case b:
    val = 17;
    break;
case 3:
    debug.assert(3 == 0);
    break;
default:
    debug.assert(1 == 0);
    break;
}

debug.assert(val, 17);

switch (i)
{
case 3:
    debug.assert(3 == 0);
    break;
default:
    val = 55;
    break;
}

debug.assert(val, 55);

function do_test()
{
    switch(1)
    {
    case 2:
	return 1;
    case 1:
	return 2;
    }
}

debug.assert(do_test(), 2);

function fallthrough_to_default()
{
    switch (1)
    {
    case 1:
	val = 3;
    default:
	return 4;
    }
}

debug.assert(fallthrough_to_default(), 4);
debug.assert(val, 3);
