/* Test module loading */
as = require('assert');
as2 = require('assert');
debug.assert(as, as);

good = 0;
try {
    doesnt_exist = require('assert2');
} catch(e) {
    good = 1;
}
debug.assert(good, 1);

therm = require('thermal_printer');

good = 0;
try {
    invalid_args = require('assert', "invalid_arg");
} catch(e) {
    good = 1;
}
debug.assert(good, 1);
