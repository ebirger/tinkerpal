var o = { age : 32, married : 1 };
debug.dump(1);
debug.dump(o);
debug.dump(2);
var l = { test : { test : { test : 2 }, test2 : 3 }, test3 : 4 };
debug.dump(l);
debug.dump(3);
debug.assert(l.test3, 4);
debug.dump(4);
debug.assert(l.test.test2, 3);
debug.dump(5);
l.test3=5;
debug.dump(6);
debug.assert(l.test3, 5);
debug.dump(7);


var empty = { };
debug.dump(empty);
debug.dump(8);
empty.test1 = 3;
debug.dump(9);
debug.assert(empty.test1, 3);
debug.dump(10);


var kuku = { };
debug.dump(11);
kuku.test1 = 4;
debug.dump(13);
debug.dump(kuku);
debug.dump(11111);

kuku.func = function() { debug.dump(2222222); return 3; };
debug.assert(kuku.func(), 3);

debug.dump("=====1====");
var ob = { value : 5 };
debug.dump("=====2====");
ob.get_value = function() { debug.dump("hahah " + this.value.toString()); return this.value; };
debug.dump("=====3====");
debug.assert(ob.get_value(), 5);
debug.dump("=====4====");
ob.get_value();
debug.dump("=====5====");
var kaplawi = function() { this.kuku = 3; debug.dump("kaplawi!"); };
//new kaplawi();
var object_with_empty_comma_in_the_end = { aa : 1, bb : 2, };
debug.assert("" + object_with_empty_comma_in_the_end, "" + { aa : 1, bb : 2 });

var a = {};
a.pre_inc = 5;
++a.pre_inc;
debug.assert(a.pre_inc, 6);
