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

good = 0;
try {
    console.log(fs.readdirSync('Builtin/kuku/ku'));
} catch (e) {
    /* Builtin fs doesn't allow subfolders */
    good = 1;
}
debug.assert(good, 1);

/* Test file handling exceptions */
good = 0;
try {
    fs.readdirSync('.', "invalid arg");
} catch (e) {
    good = 1;
}
debug.assert(good, 1);

good = 0;
try {
    fs.readFileSync('FAT/test.txt', "invalid arg");
} catch (e) {
    good = 1;
}
debug.assert(good, 1);

good = 0;
try {
    fs.readFileSync('FAT/no_such_file');
} catch (e) {
    good = 1;
}
debug.assert(good, 1);

good = 0;
try {
    fs.readdirSync('FAT/no_such_path/no_such_file');
} catch (e) {
    good = 1;
}
debug.assert(good, 1);

good = 0;
try {
    fs.writeFileSync('Local/test.txt', s, 'invalid arg');
} catch (e) {
    good = 1;
}
debug.assert(good, 1);

good = 0;
try {
    fs.readFileSync('no_such_fs/no_such_file');
} catch (e) {
    good = 1;
}
debug.assert(good, 1);

good = 0;
try {
    fs.writeFileSync('no_such_fs/test.txt', s);
} catch (e) {
    good = 1;
}
debug.assert(good, 1);

good = 0;
try {
    fs.readdirSync('no_such_fs/');
} catch (e) {
    good = 1;
}
debug.assert(good, 1);
good = 0;
try {
    fs.readdirSync('no_such_fs/');
} catch (e) {
    good = 1;
}
debug.assert(good, 1);
