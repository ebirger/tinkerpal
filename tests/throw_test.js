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

debug.assert_exception(function() { no_such_function(); });
debug.assert_exception(function() { var num = 1; num(); });
debug.assert_exception(function() { 1 + 2 * no_such_function(); });
debug.assert_exception(function() { debug.assert(1, 2, 3); });
debug.assert_exception(function() { (debug.assert(1, 2, 3)); });
debug.assert_exception(function() { (debug.assert(1, 2, 3);) });
