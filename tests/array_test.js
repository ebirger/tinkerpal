var a = [];
var b = [ 1 ];
var c = [ 1, 2 ];

var x = c[0];
a[0] = 99;
a[1] = 3;
a[2] = 4;
c[100] = "hello world";
console.log(a);
debug.assert(c[0], 1);
debug.assert(a[1], 3);
debug.assert(a[2], 4);
console.log(a.length);
debug.assert(a.length, 3);
debug.assert(b.length, 1);
console.log(c);
debug.assert(c.length, 101);
a.push(7);
debug.assert(a[3],7);
a.push(8, "hello world");
debug.assert(a[5],"hello world");
console.log(a);
a.pop();
console.log(a);
debug.assert(a.length, 5);
a.forEach(function(v) { console.log(v); });
a.forEach(function(v, k, o) { console.log(v + "[" + k + "]" + " @ " + o); });

var tt = [];
console.log([].prototype);
console.log([].forEach);
console.log([].forEach.call);
[].forEach.call(a, function (args) { tt.push(args); });
console.log("a should be equal to tt");
console.log(a);
console.log(tt);

var qq = ["god", "jesus", "budha" ];
debug.assert(qq.indexOf("god"), 0);
debug.assert(qq.indexOf("shiva"), -1);
debug.assert(qq.indexOf("budha"), 2);
console.log(qq.indexOf("budha"));

var gugu = [ "ka", "ja", "gugu", 1, 2, 3];
debug.assert(gugu.join("--"), "ka--ja--gugu--1--2--3");
debug.assert(gugu.join(), "ka,ja,gugu,1,2,3");
debug.assert(gugu.join(""), "kajagugu123");
debug.assert([].join("--"), "");

var new_arr = [1, 2, 3].map(function(x) { return x + 1; });
console.log(new_arr);
debug.assert([1, 2, 3].map(function(x) { return x + 1; })[2], 4);
debug.assert([1, 2, 3].map(function(x) { return x + 1; }).length, 3);
debug.assert([].map(function(x) { return x + 1; }).length, 0);
mapped_obj = [1, 2, 3].map(function(x, k) { return this[k] + 1; }, [ 5, 6, 7]);
console.log(mapped_obj);
debug.assert(mapped_obj[1], 7);
var a = new Array("a");
debug.assert(a[0], "a");
var ll = new Array(15);
debug.assert(ll.length, 15);
var lklk = new Array(1, 2, 3, "hello");
debug.assert(lklk[0], 1);
debug.assert(lklk[3], "hello");
var jh = Array(1, 2);
debug.assert(jh[1], 2);
var in_array_test = [ "a", "b", "c" ];
debug.assert_cond(1 in in_array_test);
debug.assert_cond(!(8 in in_array_test));

console.log([1,2,3].toString());
debug.assert([1,2,3].toString(), '1,2,3');
debug.assert([1,2,3] + 50, '1,2,350');
debug.assert([1,2,3] + 'test', '1,2,3test');
debug.assert([1,2,3] + [4,5,6], '1,2,34,5,6');

debug.assert([,].join(), "undefined,undefined");
debug.assert([1,].join(), "1,undefined");
debug.assert([,3].join(), "undefined,3");
debug.assert([1,,3].join(), "1,undefined,3");
debug.assert([1,,,3].join(), "1,undefined,undefined,3");

/* Slice */
x = [ 1, 2, 3 ].slice(1);
debug.assert(x[0], 2);
debug.assert(x[1], 3);
x = [ 1, 2, 3 ].slice(1, 2);
debug.assert(x[0], 2);
debug.assert(x[1], undefined);
x = [ 1, 2, 3, 4, 5, 6 ].slice(2, 4);
debug.assert(x.join(), "3,4");
x = [ 1, 2, 3, 4, 5, 6 ].slice(-3, 4);
debug.assert(x.join(), "4");
x = [ 1, 2, 3, 4, 5, 6 ].slice(-5, -3);
debug.assert(x.join(), "2,3");
x = [ 1, 2, 3, 4, 5, 6 ].slice(-3, -5);
debug.assert(x.join(), "");

/* Sort */
x = [3, 1, 2].sort().join();
debug.assert(x, "1,2,3");
x = [3, 1, 2].sort(function(x, y) { return x > y ? -1 : 1; }).join();
debug.assert(x, "3,2,1");
x = [3, 1, 2].sort(function(x, y) { console.log(x + ":" + y); }).join();
debug.assert(x, "3,1,2");
var y = 0;
try {
    [3, 1, 2].sort(function(x, y) { throw 1; });
    y = 2;
} catch(one) {
    debug.assert(one, 1);
}
debug.assert(y, 0);
x = [3,undefined,1].sort().join();
debug.assert(x, "1,3,undefined");
x = [];
x[3] = 5;
x.sort();
debug.assert(x.length, 4);
debug.assert(x[0], 5);
debug.assert(x[3], undefined);

x = [1, 10, 3].sort().join();
debug.assert(x, "1,10,3");
x = [1, 10, 3].sort(function(x, y) { return x - y; }).join();
debug.assert(x, "1,3,10");
