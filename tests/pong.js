console.log("Pong");

digitalWrite(GPIO_PB2, 1);
var l = new PCD8544();
var l2 = new SSD1306();
var g = new Graphics(l);
var g2 = new Graphics(l2);

function pong(gr, maxx, maxy, x, y) {
    var dx = 1, dy = 1;
    var update = compile(function() {
        gr.circleDraw(x, y, 2, 0); 
        x = x + dx;
        y = y + dy;
        gr.circleDraw(x, y, 2, 1); 
    });
    return function() { 
        update();
        if (x < 5 || x > maxx) dx *= -1;
        if (y < 5 || y > maxy) dy *= -1;
    }
}

var ball1_step = pong(g, 79, 43, 5, 5);
var ball2_step = pong(g2, 123, 59, 5, 5);

function step() {
    ball1_step();
    ball2_step();
}

console.log(compile(step));
setInterval(step, 10);
