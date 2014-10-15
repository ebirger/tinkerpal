var s = 'hello world! - uptime: ' + getTime();
console.log('Test String : ' + s);
fs.writeFileSync('Local/test.txt', s);
var s2 = fs.readFileSync('Local/test.txt');
debug.assert(s, s2);
fs.writeFileSync('FAT/test.txt', s);
var s2 = fs.readFileSync('FAT/test.txt');

var dirs = fs.readdirSync('.');
console.log(dirs);
debug.assert(dirs[0], 'FAT');
var files = fs.readdirSync('FAT/');
console.log(files);
debug.assert(files[0], 'TEST.TXT');

var dirs = fs.readdirSync('/');
debug.assert(dirs[0], 'FAT');
