console.log("==== 1");
var test = { func : function() { return { bla : 50 }; } };
console.log("==== 2");
debug.assert(test.func().bla, 50);
console.log("==== 3");
var test2 = { func : function() 
    { 
	return { blaf : function() { return { bla : 20 } } }; 
    } 
}; 
console.log("==== 4");
debug.assert(test2.func().blaf().bla, 20);
console.log("==== 5");

function recur()
{
    return function() { return function() { return function() { return 1978; }; }; };
}
console.log("==== 6");

debug.assert(recur()()()(), 1978);

console.log("==== 7");
var obj = { prop1 : 50, func : function() { return 39; } };
console.log("==== 8");
debug.assert(obj["prop1"], 50);
console.log("==== 9");
debug.assert(obj["func"](), 39);
console.log("==== 10");
debug.assert(obj.func(), 39);
console.log("==== 11");
debug.assert(obj["fu" + "nc"](), 39);
console.log("==== 12");
var obj2 = { prop1 : 50, func : function() { return { gugu : function() { return 50; } }; } };
console.log("==== 13");
debug.assert(obj2["func"]()['gugu'](), 50);
console.log("==== 14");

