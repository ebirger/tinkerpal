var n = new NetifINET();

debug.assert(n.linkStatus(), true);
debug.assert((n.MACAddrGet())[0], 0);
debug.assert((n.MACAddrGet())[0], 0);

n.IPConnect(function() {
    console.log("IP Connected: " + n.IPAddrGet());
    n.TCPConnect('188.226.224.148', 80, function() {
        console.log("TCP Connected");
        n.TCPDisconnect();
        n.IPDisconnect();
    });
});
