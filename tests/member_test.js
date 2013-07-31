debug.dump("==== 1");
var test = { func : function() { return { bla : 50 }; } };
debug.dump("==== 2");
debug.assert(test.func().bla, 50);
debug.dump("==== 3");
var test2 = { func : function() 
    { 
	return { blaf : function() { return { bla : 20 } } }; 
    } 
}; 
debug.dump("==== 4");
debug.assert(test2.func().blaf().bla, 20);
debug.dump("==== 5");

function recur()
{
    return function() { return function() { return function() { return 1978; }; }; };
}
debug.dump("==== 6");

debug.assert(recur()()()(), 1978);

debug.dump("==== 7");
var obj = { prop1 : 50, func : function() { return 39; } };
debug.dump("==== 8");
debug.assert(obj["prop1"], 50);
debug.dump("==== 9");
debug.assert(obj["func"](), 39);
debug.dump("==== 10");
debug.assert(obj.func(), 39);
debug.dump("==== 11");
debug.assert(obj["fu" + "nc"](), 39);
debug.dump("==== 12");
var obj2 = { prop1 : 50, func : function() { return { gugu : function() { return 50; } }; } };
debug.dump("==== 13");
debug.assert(obj2["func"]()['gugu'](), 50);
debug.dump("==== 14");

