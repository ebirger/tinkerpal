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
g.lineDraw(0, 0, 0, 0, 1);
console.log("draws " + num_draws);
