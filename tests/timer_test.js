var did = 0;
setTimeout(function() { debug.dump("Called"); did=1; }, 300);
setTimeout(function() { debug.assert(did, 1); }, 600);
