console.log("Pong");

var l = new PCD8544();
var g = new Graphics(l);

function pong(x, y) {
    var dx = 1, dy = 1;
    return function() { 
        x += dx; y += dy;
        if (x < 5 || x > 79) dx *= -1;
        if (y < 5 || y > 43) dy *= -1;
        g.circleDraw(x, y, 3, 1); 
    }
}

var ball1_step = pong(5, 5);
var ball2_step = pong(30, 10);
var ball3_step = pong(40, 20);

function step() {
    l.fill(0);
    ball1_step();
    ball2_step();
    ball3_step();
}
setInterval(step, 25);
