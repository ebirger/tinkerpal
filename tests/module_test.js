/* Test module loading */
as = require('assert');
as2 = require('assert');
debug.assert(as, as);


debug.assert_exception(function() { doesnt_exist = require('assert2'); });

debug.assert_exception(function() { invalid_args = require('assert', "invalid_arg"); });
