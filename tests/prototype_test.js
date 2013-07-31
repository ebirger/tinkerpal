var x = { a : "a", b : "b" };
var y = { };
var proto = { c : "c", d : "d" };
x.prototype = proto;
y.prototype = proto;

debug.dump(x);
debug.dump(y);
debug.dump(x.c);
debug.dump(y.d);

proto.prototype = { do_something : function() { return 2; } };
debug.assert(x.do_something(), 2);
proto.prototype = { do_something : function() { return 3; } };
debug.assert(x.do_something(), 3);

var conc1 = { prop : 3 }, conc2 = { prop : 4 };
var proto2 = { get_prop : function() { debug.dump(this.prop); return this.prop; } };
conc1.prototype = proto2;
conc2.prototype = proto2;
debug.assert(conc1.get_prop(), 3);
debug.assert(conc2.get_prop(), 4);
conc1.prototype.get_prop2 = function() { return this.prop + 1; };
debug.assert(conc1.get_prop2(), 4);
debug.assert(conc2.get_prop2(), 5);

function construct(age)
{
    this.papa = "Dad";
    this.mama = "Mom";
    this.age = age;
    this.kuku = function() { return 17; };
}

debug.dump("=====6====");
construct.prototype = { babi : "babi", testit : function() { debug.dump("kuku"); } };
debug.dump("=====7====");
var bla = new construct(15);
debug.dump("=====8====");
var gugu = new construct(17);
debug.dump("=====9====");

debug.assert(bla.papa, "Dad");
debug.dump("=====10====");
debug.assert(bla.kuku(), 17);
debug.dump("=====11====");
debug.assert(bla.age, 15);
debug.dump("=====12====");
debug.assert(gugu.age, 17);
debug.dump("=====13====");
debug.assert(gugu.mama, "Mom");
debug.dump("=====14====");
debug.assert((new construct(33)).age, 33);
debug.dump("=====15====");
debug.dump(construct);
debug.dump("=====16====");
debug.dump(bla);
debug.dump("=====17====");
bla.testit();
debug.assert(bla.babi, "babi");
debug.dump("=====18====");
