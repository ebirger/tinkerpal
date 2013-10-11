var a = new ArrayBuffer(16);
debug.assert(a.byteLength, 16);
var k = new Int8Array(a);
k[3] = 5;
k[3] += 8;
debug.assert(k[3], 13);
