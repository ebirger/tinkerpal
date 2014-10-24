var s = new Serial(UART0);

s.print('Hello!');
s.write('Hello!');
s.write(65);
s.write(['hello1\n','hello2\n','hello3\n']);

debug.assert_exception(function() { console.set(); });
debug.assert_exception(function() { console.log(); });

console.set(s);
s.onData(function(e) { s.onData(); console.log(e); });

var s2 = new Serial(UART3);
s2.print('kuku!');
