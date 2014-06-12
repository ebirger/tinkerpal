function k() {
    debug.assert(arguments.length, 4);
    debug.assert(arguments[0], 1);
    debug.assert(arguments[1], 2);
    debug.assert(arguments[2], 3);
    debug.assert(arguments[3], 4);
    console.log(arguments);
}

k(1, 2, 3, 4);

function p() {
    k(1, 2, 3, 4);
}

p(2, 3, 4, 5);

b = 3;
function f(a, b, c) { debug.assert(b, undefined); }
f(1);
