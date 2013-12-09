var o = { age : 32, married : 1 };
console.log(1);
console.log(o);
console.log(2);
var l = { test : { test : { test : 2 }, test2 : 3 }, test3 : 4 };
console.log(l);
console.log(3);
debug.assert(l.test3, 4);
console.log(4);
debug.assert(l.test.test2, 3);
console.log(5);
l.test3=5;
console.log(6);
debug.assert(l.test3, 5);
console.log(7);


var empty = { };
console.log(empty);
console.log(8);
empty.test1 = 3;
console.log(9);
debug.assert(empty.test1, 3);
console.log(10);


var kuku = { };
console.log(11);
kuku.test1 = 4;
console.log(13);
console.log(kuku);
console.log(11111);

kuku.func = function() { console.log(2222222); return 3; };
debug.assert(kuku.func(), 3);

console.log("=====1====");
var ob = { value : 5 };
console.log("=====2====");
ob.get_value = function() { console.log("hahah " + this.value.toString()); return this.value; };
console.log("=====3====");
debug.assert(ob.get_value(), 5);
console.log("=====4====");
ob.get_value();
console.log("=====5====");
var kaplawi = function() { this.kuku = 3; console.log("kaplawi!"); };
//new kaplawi();
var object_with_empty_comma_in_the_end = { aa : 1, bb : 2, };
debug.assert("" + object_with_empty_comma_in_the_end, "" + { aa : 1, bb : 2 });

var a = {};
a.pre_inc = 5;
++a.pre_inc;
debug.assert(a.pre_inc, 6);

var in_obj_test = { a : 1, b : 2 };
debug.assert_cond("a" in in_obj_test);
debug.assert_cond(!("c" in in_obj_test));

function o() { this.a = 1; this.b = { aa : this.a }; }
var oo = new o();
debug.assert(oo.b.aa, 1);

var string_prop = { "a" : 3 };
debug.assert(string_prop.a, 3);
