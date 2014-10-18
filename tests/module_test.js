/* Test module loading */
as = require('assert');
as2 = require('assert');
debug.assert(as, as);
as.equal(1, 1, "Ok");

good = 0;
try {
    doesnt_exist = require('assert2');
} catch(e) {
    good = 1;
}
debug.assert(good, 1);

therm = require('thermal_printer');
