var x = 0;

function bla()
{
    throw 1;
}

function kuku()
{
    debug.dump(1);
    bla();
    debug.assert(2, 0);
}

function dudu()
{
    debug.dump(2);
    kuku();
    debug.assert(1, 0);
}

function pupu()
{
    debug.dump(3);
    try 
    {
	debug.dump(4);
	dudu();
	debug.assert(3, 0);
    }
    catch (should_be_one)
    {
	var x = 1 + 1;
	debug.dump("kuku");
	debug.assert(should_be_one, 1);
    }
}

function susu()
{
    debug.dump(6);
    pupu();
    x = 1;
}

susu();
debug.assert(x, 1);

try
{
    no_such_function();
}
catch(s)
{
    debug.dump(s);
}

try
{
    var num = 1;
    num();
}
catch(s)
{
    debug.dump(s);
}

try
{
    1 +  2 * no_such_function();
}
catch(s)
{
    debug.dump(s);
}
