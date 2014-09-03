var did = 0;
setTimeout(function() { console.log("Called"); did=1; }, 300);
setTimeout(function() { debug.assert(did, 1); }, 600);

var count = 0;
var tid = setInterval(function() { if (++count == 3) clearInterval(tid); }, 300);
setTimeout(function() { debug.assert(count, 3); }, 1200);
var count2 = 0;
var tid2 = setInterval(function() { if (++count2 == 3) return -1; }, 300);
setTimeout(function() { debug.assert(count2, 3); }, 1200);
setTimeout(function() { }, 1600);
setTimeout(function() {clearTimeout(); }, 1500);
