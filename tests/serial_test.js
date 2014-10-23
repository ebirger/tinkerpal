var s = new Serial(UART0);

s.print('Hello!');
s.write('Hello!');
s.write(65);
s.write(['hello1\n','hello2\n','hello3\n']);

good = 0;
try {
    console.set();
} catch(e) {
    good = 1;
}
debug.assert(good, 1);
good = 0;
try {
    console.log();
} catch(e) {
    good = 1;
}
debug.assert(good, 1);

console.set(s);
s.onData(function(e) { s.onData(); console.log(e); });
