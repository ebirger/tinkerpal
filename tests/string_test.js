console.log("test");
var x = "hello world";
console.log(x);
console.log('test2');
var a = "val_a", b = "val_b", c = "val_a";
debug.assert(a == b, false);
debug.assert(a == c, true);
var foo = "foo", bar = "bar", foobar = "foobar", out;
out = foo + bar;
debug.assert(out, foobar);
var three = 3;
console.log("three=" + three.toString() + "!");
var obj = { g : "g" };
console.log("obj=" + obj.toString());
obj.toString = function() { return "toStringOverride!"; };
console.log("obj=" + obj.toString());
obj.bla = 3;
var s = "kuku";
s+="poo";
console.log(s);
debug.assert(s, "kukupoo");
s = "kuku---gug---bub";
console.log(s.prototype);
console.log(s.split);
console.log(s.split("---"));
debug.assert(s.split("---")[1], "gug");
debug.assert("kuku".split("")[2], "k");
console.log("kukugug".split(""));
debug.assert("".split(",")[0], "");
debug.assert("kuku".split()[0], "kuku");
console.log("".split());
debug.assert("kukuya".length, 6);
debug.assert("kaku".indexOf("ku"), 2);
debug.assert("kaku".indexOf(), -1);
debug.assert("012345678".substring(2, 7), "23456");
debug.assert("012345678".substring(2), "2345678");
console.log("012345678".substring(2,5));
debug.assert("012345678".charAt(4), "4");
debug.assert("123\n".length, 4);
console.log("123\n123");
debug.assert("\n", "\u000a");
var lui = new String("lui");
debug.assert(lui, "lui");
var fui = String("fui");
debug.assert(fui, "fui");
debug.assert("abc"[1], "b");
debug.assert("abc"[4], undefined);
x = "abc";
x[4] = 1;
debug.assert("abc"[4], undefined);
debug.assert([].map.call("abc", function(x) { return x + 1; })[1], "b1");

/* Upper | Lower case */
debug.assert("kuku".toUpperCase(), "KUKU");
debug.assert("ku123Lku".toUpperCase(), "KU123LKU");
debug.assert("KUKU".toLowerCase(), "kuku");
debug.assert("ku123Lku".toLowerCase(), "ku123lku");
debug.assert("KUKU".toLowerCase(1), "kuku");

/* Implicit casting */
debug.assert(undefined + "", "undefined");
debug.assert("" + undefined, "undefined");
debug.assert("" + 3, "3");
debug.assert(3 + "", "3");
debug.assert(true + "", "true");

/* charCodeAt() */
debug.assert("a string".charCodeAt(0), 97);
debug.assert("a string".charCodeAt(3), 116);

/* String unescaping */
debug.assert("\r".charCodeAt(0),13);
debug.assert("\t".charCodeAt(0),9);
debug.assert("\\".charCodeAt(0),92);
debug.assert("\0".charCodeAt(0),0);
debug.assert("\'".charCodeAt(0),39);
debug.assert("\"".charCodeAt(0),34);

/* String comparison */
debug.assert("" > "", false);
debug.assert("" < "", false);
debug.assert("" >= "", true);
debug.assert("" <= "", true);
debug.assert("1" > "3", false);
debug.assert("10" > "3", false);
debug.assert("10" > "1", true);
debug.assert("10" < "3", true);
debug.assert("2" < "3", true);
debug.assert("100" < "1000", true);
debug.assert("10" >= "10", true);
debug.assert("10" <= "10", true);
debug.assert("10" == "10", true);
debug.assert("10" != "11", true);
debug.assert("20" != "22", true);
debug.assert("" == "1", false);
debug.assert("1" == "", false);

/* Number to string */
debug.assert((1).toString(), '1');
debug.assert((3).toString(2), '11');
debug.assert((5).toString(16), '5');
debug.assert((5).toString(4), '11');
debug.assert((16).toString(16), '10');
debug.assert((10).toString(16), 'a');
debug.assert((33).toString(16), '21');
