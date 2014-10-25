var j;

j = eval("1 + 2");
debug.assert(j, 3);

var k;

k = eval("j = { foo : 43 }");
debug.assert(k.foo, 43);

var l = eval("1" + "+" + "1");
debug.assert(l, 2);

var m = eval(1);
debug.assert(m, 1);

debug.assert(eval(), undefined);
