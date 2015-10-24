var n = new NetifINET();

/* Test method call on non netif object */
debug.assert_exception(function() { n.linkStatus.call(1); });
debug.assert_exception(function() { n.MACAddrGet.call(1); });
debug.assert_exception(function() { n.IPAddrGet.call(1); });
debug.assert_exception(function() { n.IPConnect.call(1); });
debug.assert_exception(function() { n.IPDisconnect.call(1); });
debug.assert_exception(function() { n.TCPDisconnect.call(1); });
debug.assert_exception(function() { n.onTCPData.call(1); });
debug.assert_exception(function() { n.onTCPDisconnect.call(1); });
debug.assert_exception(function() { n.TCPWrite.call(1, "kku"); });
debug.assert_exception(function() { n.TCPRead.call(1, "kku"); });
debug.assert_exception(function() { n.TCPRead.call(1); });

/* Test invalid IP */
good = 0;
debug.assert_exception(function() { n.TCPConnect('188.226.224148', 80, function() { }); });
/* Test invalid IP */
debug.assert_exception(function() { n.TCPConnect('1a88.226.224.148', 80, function() { }); });

debug.assert(n.linkStatus(), true);
debug.assert((n.MACAddrGet())[0], 0);
debug.assert((n.MACAddrGet())[0], 0);

function do_weather() {
    var full = "";
    n.onTCPDisconnect(function() {
        n.TCPDisconnect();
        n.IPDisconnect();
        n.onTCPData();
        console.log(full);
    });
    n.onTCPData(function() {
        var s = n.TCPRead();
        if (s == "") {
            return;
        }
        full += s;
    });
    n.TCPConnect('173.194.116.144', 80, function() {
        console.log("TCP Connected");
        n.TCPWrite('GET / HTTP/1.0\r\n' +
            '\r\n\r\n');
    });
}

n.IPConnect(function() {
    console.log("IP Connected: " + n.IPAddrGet());
    do_weather();
});
