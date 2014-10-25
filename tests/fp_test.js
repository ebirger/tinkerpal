debug.assert(345,345);
debug.assert(34.5*10,345);
debug.assert(3.45*100,345);
debug.assert(3.45e2,345);
debug.assert(0531,345);
debug.assert(0xdeadbeaf,3735928495);
debug.assert_exception(function() { var x = 3.45e; });
debug.assert_exception(function() { var x = 3.45e2e3; });
