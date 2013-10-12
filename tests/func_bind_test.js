function template() { return this.prop; }
var t = template.bind({ prop : 1 });
debug.assert(t(), 1);
