var n = new NetifINET();

debug.assert(n.linkStatus(), true);
debug.assert((n.MACAddrGet())[0], 0);
debug.assert((n.MACAddrGet())[0], 0);

n.IPConnect(function() {
    console.log("IP Connected: " + n.IPAddrGet());
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
    n.TCPConnect('188.226.224.148', 80, function() {
        console.log("TCP Connected");
        n.TCPWrite('GET /data/2.5/weather?q=New%20York,US HTTP/1.0\r\n' +
            'Host: api.openweathermap.org\r\n\r\n');
    });
});
