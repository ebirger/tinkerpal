function template() { return this.prop; }
var t = template.bind({ prop : 1 });
debug.assert(t(), 1);
var v = 1;
function f() {return this.v; }
var k = f.bind();
debug.assert(k(), v);
