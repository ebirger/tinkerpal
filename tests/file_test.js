var s = 'hello world! - uptime: ' + getTime();
console.log('Test String : ' + s);
fs.writeFileSync('Local/test.txt', s);
var s2 = fs.readFileSync('Local/test.txt');
debug.assert(s, s2);
fs.writeFileSync('FAT/test.txt', s);
var s2 = fs.readFileSync('FAT/test.txt');
