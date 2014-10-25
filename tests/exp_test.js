console.log(1);
debug.assert(1, 1);
debug.assert(!1, false);
debug.assert(!0, true);
console.log(2);
debug.assert(~0, -1);
console.log(3);
debug.assert(~1, -2);
debug.assert(1*1,1);
console.log(3);
debug.assert(1*2,2);
console.log(4);
debug.assert(2*2,4);
debug.assert(1+2*2+1,6);
debug.assert((1+2)*2+2,8);
debug.assert((1+-2)*2,-2);
console.log("===========");
console.log(2/2*2);
debug.assert(2/2*2,2);
console.log("===========");
debug.assert(1+2/2+5*2,12);
console.log("===========");
debug.assert((2+2)/2,2);
debug.assert(5%3,2);
debug.assert(1+5%3,3);
debug.assert((1+5)%3,0);
debug.assert(1<<0,1);
debug.assert(1<<1,2);
debug.assert(3<<1,6);
debug.assert(1>>0,1);
debug.assert(1>>1,0);
debug.assert(3>>1,1);
debug.assert(1+2<<1+3,48);
debug.assert(2*1024>>2*3,32);
console.log("===========");
debug.assert(1<1,false);
debug.assert(1<0,false);
debug.assert(1<-1,false);
console.log("===========");
debug.assert(1<2,true);
debug.assert(1<2<3,true);
console.log("===========");
debug.assert(1>1,false);
console.log("===========");
debug.assert(2>1,true);
debug.assert(2>1>0,true);
debug.assert(5>3>2,false);
debug.assert(1>=1,true);
debug.assert(1<=1,true);
debug.assert(1>=2,false);
debug.assert(2>=1,true);
debug.assert(1+3<2+5,true);
debug.assert(1*3+5<2+5,false);
debug.assert(1==1,true);
debug.assert(1==0,false);
debug.assert(1!=0,true);
debug.assert(!!!1,false);
debug.assert(!(1==0),true);
debug.assert(1&&1||2, true);
debug.assert(1&&1||2*0, true);
debug.assert(0||1, true);
var x = 0, y, z, q;
console.log(3);
debug.assert(x++,0);
console.log(5);
debug.assert(++x,2);
console.log(6);
debug.assert(x,2);
console.log(7);
x = 1;
console.log(8);
y = 0;
y+= x += 5;
debug.assert(x, 6);
console.log(4);
debug.assert(y, 1);
console.log(4);
x = 0;
y = 0 && x++;
debug.assert(y, 0);
debug.assert(x, 0);
console.log(5);
y = 1 || x++;
console.log(7);
debug.assert(x, 0);
console.log(113);
debug.assert(y, 1);
console.log(113);
z = 0;
console.log(113);
y = x++ && z++;
console.log(113);
debug.assert(x,1);
console.log(113);
debug.assert(z,0);
y = x++ && z++;
console.log(112);
debug.assert(x,2);
debug.assert(z,1);
debug.assert(1 ? 1 : 0, 1);
debug.assert(0 ? 1 : 0, 0);
console.log(111);
y = 1;
x = y ? 3 : 5;
debug.assert(x,3);
console.log(111);
console.log(y);
z = y == 5 ? 3 : 2;
console.log(115);
debug.assert(z, 2);
console.log(116);
q = z ? 1 + 2 * 12 : 222;
console.log(117);
debug.assert(q, 25);
console.log(118);
y = 0;
console.log(119);
x = 1 ? 2 : y += 2;
console.log(120);
debug.assert(x,2);
console.log(121);
debug.assert(y, 0);
console.log(122);
x = 1 ? 2 : y += 2;
console.log(123);
debug.assert(x, 2);
console.log(124);
debug.assert(y, 0);
x = 0 ? 2 : (y += 2);
debug.assert(x, 0);
debug.assert(y, 2);
x = 1 ? 2 ? 1 : 3 : 2;
debug.assert(x, 1);
x = ((1+3)*(((((2 + 5)*2)*2)*2)*2));
debug.assert(x, 448);
var qq = 3, ll = (5 + 12);
debug.assert(qq, 3);
debug.assert(ll, 17);
var t1 =3, t2;
t2 = t1;
t2++;
debug.assert(t1,3);
debug.assert(t1,3);
console.log("+++++++++++++++++++++++");
var jj = 1;
var yy;

debug.assert(1^1, 0);
debug.assert(1^0, 1);
debug.assert(0^1, 1);
debug.assert(0^0, 0);

var a = 0b1100;
a^=0b1111;
debug.assert(a, 0b0011);

yy = 1 == 1 ? jj++ : 0;
debug.assert(yy, 1);
debug.assert(jj, 2);
debug.assert_cond(1 == true);
debug.assert_cond(1 !== true);

/* SHRZ */
console.log("++++++SHRZ +++++++++");

debug.assert(-1 >> 16, -1);
debug.assert(-1 >>> 16, 0xFFFF);

var x = 1;
x <<= 2;
debug.assert(x, 4);

x = -1;
x >>= 1;
debug.assert(x, -1);
x >>>= 1;
debug.assert(x, 2147483647);

/* toInteger */
debug.assert(toInteger(), 0);
debug.assert(toInteger(1), 1);
debug.assert(toInteger("1"), 1);
debug.assert(toInteger(-1), -1);
debug.assert(toInteger("-1"), -1);
debug.assert(toInteger(1.1), 1);
debug.assert(toInteger(1.9), 1);
debug.assert(toInteger(-1.9), -1);
debug.assert(toInteger("-1.9"), -1);

/* Multiple ternary expressions */
debug.assert(0?0:1+0?1:2+3?1:0,1);

/* Large numbers */
debug.assert((1<<30) + 1, 1073741825);

/* Integer literals */
debug.assert(0x12, 18);
debug.assert(0X12, 18);
debug.assert(-0x12, -18);
debug.assert(012, 10);
debug.assert(-012, -10);
debug.assert(0o12, 10);
debug.assert(0O12, 10);
debug.assert(0b110, 6);
debug.assert(-0b110, -6);
debug.assert(-0B110, -6);

/* String Multiplicative Operators */
debug.assert("123"*3, 369);
debug.assert("72"/3, 24);
debug.assert("73"%3, 1);

/* String Subtraction Operator */
debug.assert("73"-3, 70);

/* isNaN */
debug.assert(isNaN(), true);
debug.assert(isNaN(1), false);
debug.assert(isNaN("kuku"), true);
debug.assert(isNaN(""), false); // converted to 0
debug.assert(isNaN("  "), false); // converted to 0
debug.assert(isNaN("123"), false);
debug.assert(isNaN(" 123"), false);
debug.assert(isNaN(" 123.0"), false);
debug.assert(isNaN({}), true);
debug.assert(isNaN(true), false);
debug.assert(isNaN(false), false);
debug.assert(isNaN(undefined), true);
debug.assert(isNaN(null), false);

/* FP <-> Int */
debug.assert(0|0.1, 0);
debug.assert(0|0.9, 0);
debug.assert(0|1.9, 1);
debug.assert(1&1.9, 1);
debug.assert(1&0.9, 0);
debug.assert(1^0.9, 1);
debug.assert(1^1.9, 0);

/* Expression skipping tests */
debug.assert(true || x.y == 1, true);
debug.assert(true || x['kuku'] == 3, true);
debug.assert(true || dummy(arg1, arg2), true);
debug.assert(true || [dummy1, dummy2], true);

/* FP operators */
var f = 1.3;
f++;
debug.assert(f, 2.3);
f++;
f++;
f--;
debug.assert(f, 3.3);
console.log(f);
debug.assert(3.2/4, 0.8);
debug.assert(3.2/4 > 1, false);
debug.assert(3.2/4 < 1, true);
debug.assert(3.2/3 >= 1, true);
debug.assert(3.2/3 <= 1, false);
debug.assert(3.2/3 != 1, true);
debug.assert(undefined > 3, false);
debug.assert(undefined < 3, false);
debug.assert(undefined <= 3, false);
debug.assert(undefined >= 3, false);
debug.assert(3 > undefined, false);
debug.assert(3 < undefined, false);
debug.assert(3 >= undefined, false);
debug.assert(3 <= undefined, false);
debug.assert(undefined > undefined, false);
debug.assert(undefined < undefined, false);
debug.assert(undefined <= undefined, false);
debug.assert(undefined >= undefined, false);
debug.assert(undefined == undefined, true);
debug.assert(undefined === undefined, true);
debug.assert(undefined == null, true);
debug.assert(undefined === null, false);
debug.assert(undefined != null, false);
debug.assert(undefined !== null, true);
debug.assert(null == null, true);
debug.assert(null === null, true);
debug.assert(null != null, false);
debug.assert(null !== null, false);
debug.assert(null == undefined, true);
debug.assert(null === undefined, false);
debug.assert(null != undefined, false);
debug.assert(null !== undefined, true);

/* Binary operators */
debug.assert(1|2, 3);
debug.assert(1&3, 1);
debug.assert(10000000|20000000, 28950400);
debug.assert(1&3, 1);

debug.assert_exception(function() { var x = 0x123g; });
debug.assert_exception(function() { var x = 1.3.3; });
