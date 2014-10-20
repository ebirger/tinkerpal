var x = 0;

function bla()
{
    throw 1;
}

function kuku()
{
    console.log(1);
    bla();
    debug.assert(2, 0);
}

function dudu()
{
    console.log(2);
    kuku();
    debug.assert(1, 0);
}

function pupu()
{
    console.log(3);
    try 
    {
	console.log(4);
	dudu();
	debug.assert(3, 0);
    }
    catch (should_be_one)
    {
	var x = 1 + 1;
	console.log("kuku");
	debug.assert(should_be_one, 1);
    }
}

function susu()
{
    console.log(6);
    pupu();
    x = 1;
}

susu();
debug.assert(x, 1);

good = 0;
try
{
    no_such_function();
}
catch(s)
{
    console.log(s);
    good = 1;
}
debug.assert(good, 1);

good = 0;
try
{
    var num = 1;
    num();
}
catch(s)
{
    console.log(s);
    good = 1;
}
debug.assert(good, 1);

good = 0;
try
{
    1 +  2 * no_such_function();
}
catch(s)
{
    console.log(s);
    good = 1;
}
debug.assert(good, 1);

good = 0;
try
{
    /* Invalid args */
    debug.assert(1, 2, 3);
}
catch(s)
{
    console.log(s);
    good = 1;
}
debug.assert(good, 1);
