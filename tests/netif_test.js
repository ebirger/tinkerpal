var n = new NetifINET();

debug.assert(n.linkStatus(), true);
debug.assert((n.MACAddrGet())[0], 0);
debug.assert((n.MACAddrGet())[0], 0);

n.IPConnect(function() {
    console.log("IP Connected: " + n.IPAddrGet());
    n.IPDisconnect();
});
