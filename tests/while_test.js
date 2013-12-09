var x, y;
function bla()
{
    x = 3;
    while (--x)
    {
	if (x > 1)
	    continue;

	return 5;
    }
    debug.assert(1,0);
}
function main()
{
    x = 5;
    while (x--);
    debug.assert(x, -1);
    
    x = 5;
    while (--x);
    debug.assert(x, 0);
    x = 3;
    while (x)
	x--;
    debug.assert(x, 0);

    y = 5;
    x = 7;
    while (--x)
    {
	if (x > 3)
	    continue;
	y--;
    }
    debug.assert(x, 0);
    debug.assert(y, 2);
    debug.assert(bla(),5);
    x = 5;
    while (--x)
    {
	if (x == 3)
	{
	    break;
	}
    }
    debug.assert(x, 3);
}

var i = 10;

do console.log(i); while(--i);

debug.assert(i, 0);

var a = 0;
do { a++; } while (a < 10);

debug.assert(a, 10);

var a = 0;
do
{
    a++;
    if (a > 5)
	break; 
} while (a < 10);

debug.assert(a, 6);

var a = 0, c = 0;
do
{
    a++;
    if (a < 5)
	continue; 
    c++;
} while (a < 10);

debug.assert(c, 6);

main();
