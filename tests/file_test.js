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

var builtin_files = fs.readdirSync('Builtin');
debug.assert(builtin_files[0], 'assert');

/* Builtin fs doesn't allow subfolders */
debug.assert_exception(function() { console.log(fs.readdirSync('Builtin/kuku/ku')); });

/* Test file handling exceptions */
debug.assert_exception(function() { fs.readdirSync('.', "invalid arg"); });
debug.assert_exception(function() { fs.readFileSync('FAT/test.txt', "invalid arg"); });
debug.assert_exception(function() { fs.readFileSync('FAT/no_such_file'); });
debug.assert_exception(function() { fs.readdirSync('FAT/no_such_path/no_such_file'); });
debug.assert_exception(function() { fs.writeFileSync('Local/test.txt', s, 'invalid arg'); });
debug.assert_exception(function() { fs.readFileSync('no_such_fs/no_such_file'); });
debug.assert_exception(function() { fs.writeFileSync('no_such_fs/test.txt', s); });
debug.assert_exception(function() { fs.readdirSync('no_such_fs/'); });
debug.assert_exception(function() { fs.readdirSync('no_such_fs/'); });
