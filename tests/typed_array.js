/* Assuming little endian */
var a = new ArrayBuffer(16);
debug.assert(a.byteLength, 16);
var k = new Int8Array(a);
var k16 = new Int16Array(a);
k[3] = 5;
k[3] += 8;
debug.assert(k[3], 13);
debug.assert(k16[1], 3328);

var a = new ArrayBuffer(16);
var k32 = new Int32Array(a);

for (var i=0; i<k32.length; i++) 
{
  k32[i] = i*2;
}

var k16 = new Int16Array(a);

debug.assert(k16[0],0);
debug.assert(k16[1],0);
debug.assert(k16[2],2);
debug.assert(k16[3],0);
debug.assert(k16[4],4);
debug.assert(k16[5],0);
debug.assert(k16[6],6);
debug.assert(k16[7],0);
k16[0] = 32;
debug.assert(k32[0], 32);

var k = new Int8Array(new ArrayBuffer(4));

for (var i = 0; i < 4; i++)
    k[i] = i;
var arr = [].map.call(k, function(x) { return x; });
debug.assert(arr.length, 4);
debug.assert(arr[3], 3);

var k = new Int8Array(8);
k[3] = 5;
debug.assert(k[3],5);
var k16 = new Int16Array(8);
k16[6] = 5;
debug.assert(k16[6],5);
var k32 = new Int32Array(8);
k32[6] = 5;
debug.assert(k32[6],5);

debug.assert(k.BYTES_PER_ELEMENT, 1);
debug.assert(k16.BYTES_PER_ELEMENT, 2);
debug.assert(k32.BYTES_PER_ELEMENT, 4);

var k = new Int8Array(16);
k[14]++;
debug.assert(k[14],1);
++k[14];
debug.assert(k[14],2);

var k32 = new Uint32Array(64);
debug.assert(k32.length, 64);
for (var i = 0; i < k32.length; i++) k32[i] = (i+1)*2;

var s32 = k32.subarray(1);
debug.assert(s32[0], k32[1]);
debug.assert(s32.length, k32.length - 1);
s32 = k32.subarray(1, -1);
debug.assert(s32[61], k32[62]);
debug.assert(s32.length, k32.length - 2);
s32_2 = s32.subarray(1, -1);
debug.assert(s32_2.length, k32.length - 4);
debug.assert(s32_2[0], k32[2]);
debug.assert(s32_2[59], k32[61]);
s32_3 = s32.subarray(3, 5);
debug.assert(s32_3[1], s32[4]);

var a = new ArrayBuffer(16);
var k8 = new Int8Array(a, 0, 16);
var k8_2 = new Int8Array(a, 1, 5);
for (var i = 0; i < 16; i++)
    k8[i] = (i + 1) * 2;
for (var i = 0; i < 5; i++)
    debug.assert(k8_2[i], k8[i+1]);

var a = new Int8Array(16);
a[5] = 3;
var b = new Int16Array(a);
debug.assert(b[5], 3);
a[3] = 5;
debug.assert(b[3], 0);
debug.assert(a.length, b.length);

var a = new Uint16Array([1, 2, 3]);
debug.assert(a.length, 3);
debug.assert(a[0], 1);
debug.assert(a[1], 2);
debug.assert(a[2], 3);
