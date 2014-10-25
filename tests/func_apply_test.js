function sum(a, b) { return a+b; }
console.log("==1==");
debug.assert(sum.apply(undefined, [1,2]), 3);
function add_prop() { this.prop = 1; }
var o = {};
add_prop.apply(o);
console.log("==2==");
debug.assert(o.prop, 1);
function this_sum(a, b) { this.sum = a + b; }
this_sum.apply(o, [3, 5]);
console.log("==3==");
debug.assert(o.sum, 8);
/* Test non-array argsArray */
console.log("==4==");
debug.assert(sum.apply(undefined, "abc"), "ab");
debug.assert_exception(function() { sum.apply(); });
debug.assert_exception(function() {
    /* Test too many args */
    sum.apply(undefined,[1,2,3,4,5,6,7,8,9,10,11,12,13,14]); 
});
