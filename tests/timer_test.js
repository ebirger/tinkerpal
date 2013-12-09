var did = 0;
setTimeout(function() { console.log("Called"); did=1; }, 300);
setTimeout(function() { debug.assert(did, 1); }, 600);
