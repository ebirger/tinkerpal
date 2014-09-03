var did = 0;
setTimeout(function() { console.log("Called"); did=1; }, 300);
setTimeout(function() { debug.assert(did, 1); }, 600);

var count = 0;
var tid = setInterval(function() { if (++count == 3) clearInterval(tid); }, 300);
setTimeout(function() { debug.assert(count, 3); }, 1200);
