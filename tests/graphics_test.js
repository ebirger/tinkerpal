var num_draws = 0;
var num_flips = 0;

var c = {
    pixelDraw: function(x, y, c) { num_draws++; },
    flip: function() { num_flips++; },
    height: 200,
    width: 320
};
var g = new Graphics(c);

g.stringDraw(0, 0, "Hello!", 1);
console.log("draws " + num_draws);
c.flip();
debug.assert(num_flips, 1);
g.circleDraw(0, 0, 5, 1);
console.log("draws " + num_draws);
g.circleFill(0, 0, 5, 1);
console.log("draws " + num_draws);
g.lineDraw(0, 0, 5, 1, 1);
console.log("draws " + num_draws);
g.rectDraw(0, 0, 5, 1, 1);
console.log("draws " + num_draws);
g.rectFill(0, 0, 5, 1, 1);
console.log("draws " + num_draws);
g.roundRectDraw(0, 0, 5, 1, 1, 1);
console.log("draws " + num_draws);
g.roundRectFill(0, 0, 5, 1, 1, 1);
console.log("draws " + num_draws);
g.lineDraw(0, 0, 0, 0, 1);
console.log("draws " + num_draws);

debug.assert_exception(function() { var g = new Graphics()});
debug.assert_exception(function() { var g = new Graphics({})});
/* Test invalid params */
debug.assert_exception(function() { g.stringDraw(); } );
debug.assert_exception(function() { var s = g.stringDraw; s(1, 1, "", 1); } );
debug.assert_exception(function() { g.circleDraw(); } );
debug.assert_exception(function() { var s = g.circleDraw; s(0, 0, 5, 1); } );
debug.assert_exception(function() { g.circleFill(); } );
debug.assert_exception(function() { var s = g.circleFill; s(0, 0, 5, 1); } );
debug.assert_exception(function() { g.lineDraw(); } );
debug.assert_exception(function() { var s = g.lineDraw; s(0, 0, 5, 1, 1); } );
debug.assert_exception(function() { g.roundRectDraw(); } );
debug.assert_exception(function() { var s = g.roundRectDraw; s(0, 0, 5, 1, 1, 1); } );
debug.assert_exception(function() { g.roundRectFill(); } );
debug.assert_exception(function() { var s = g.roundRectFill; s(0, 0, 5, 1, 1, 1); } );
debug.assert_exception(function() { g.rectDraw(); } );
debug.assert_exception(function() { var s = g.rectDraw; s(0, 0, 5, 1, 1); } );
debug.assert_exception(function() { g.rectFill(); } );
debug.assert_exception(function() { var s = g.rectFill; s(0, 0, 5, 1, 1); } );

/* Test canvas operations */
var d = new DummyCanvas();
d.flip();
d.fill(1);
d.pixelDraw(1, 1, 1);
var dummy_g = new Graphics(d);
debug.assert_exception(function() { d.fill(); } );
debug.assert_exception(function() { var s = d.fill; s(1); } );
debug.assert_exception(function() { d.pixelDraw(); } );
debug.assert_exception(function() { var s = d.pixelDraw; s(0, 0, 5); } );
debug.assert_exception(function() { d.flip(1); } );
debug.assert_exception(function() { var s = d.flip; s(); } );
