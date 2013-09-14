var i;

for (i = 0; i < 20; i+=2)
	debug.dump(i);
debug.assert(i, 20);

for (i = 0; i < 20; i++)
{
    if (i == 5)
	break;
}

debug.assert(i, 5);

var x, y, z = 0;

for (x = 0; x < 10; x++)
{
    for (y = 0; y < 10; y++)
    {
	if (y == 5)
	    break;

	z++;
    }
}

debug.assert(z, 50);

var i, a = [4, 5, 6], sum = 0;
for (i in a) 
    sum += a[i];
debug.assert(sum, 15);
for (i in a) 
    sum += a[i];
debug.assert(sum, 30);
for (i in a)
{
    if (i == 1)
	break;
    sum += a[i];
}
debug.assert(sum, 34);
