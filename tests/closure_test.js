var basic_closure = function() {
    var count = 0;
    return function() {
        return ++count;
    };
};
var inc = basic_closure();
debug.assert(inc(), 1);
debug.assert(inc(), 2);
debug.assert(inc(), 3);
debug.assert(inc(), 4);

var limited_scoping = function() {
    return function() {
	var x;

	x = 1;

	return x;
    };
};

var t = limited_scoping();
debug.assert(t(), 1);
debug.assert(t(), 1);

var sum_factory = function() {
    return function(x, y) {
	return x + y;
    };
};

var sum = sum_factory();
debug.assert(sum(1, 2), 3);
