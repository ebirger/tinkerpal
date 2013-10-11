var a = new ArrayBuffer(16);
debug.assert(a.byteLength, 16);
var k = new Int8Array(a);
var k2 = new Int16Array(a);
k[3] = 5;
k[3] += 8;
debug.assert(k[3], 13);
debug.assert(k2[1], 3328); /* Assuming little endian */
