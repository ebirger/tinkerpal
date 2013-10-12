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
