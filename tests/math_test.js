/* Most of these checks pass fine, though comparing FPs is risky */
debug.assert(Math.sin(Math.Pi / 2), 1);
c = Math.cos(Math.Pi / 2);
debug.assert_cond(c > -0.005 && c < 0.005);
debug.assert(Math.asin(1), Math.Pi / 2);
debug.assert(Math.acos(1), 0);
debug.assert(Math.tan(Math.Pi / 4), 1);
debug.assert(Math.atan(0), 0);
debug.assert(Math.sqrt(9), 3);
debug.assert(Math.log(Math.E * Math.E), 2);
debug.assert(Math.exp(2), Math.E * Math.E);
debug.assert(Math.floor(2.4), 2);
debug.assert(Math.ceil(2.4), 3);
debug.assert(Math.round(2.4), 2);
debug.assert(Math.round(2.6), 3);
debug.assert(Math.abs(-2), 2);
debug.assert(Math.atan2(1, 1), Math.Pi / 4);
debug.assert(Math.pow(10, 2), 100);
