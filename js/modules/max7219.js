/* Copyright (c) 2014 Eyal Birger, See LICENSE file for details.
 * MAX7219 8 Digit LED Display Driver
 *
 * Connections: VCC, GND, DIN, CLK, CS
 *
 * Usage:
 *   var max = require('max7219');
 *   var m = new max.instance(new SPI(SPI1), GPIO_PF2);
 *   m.set("1234HELP");
 *
 * Dots are enabled using a bitmap in the second argument, e.g.:
 *
 *   m.set("1234HELP", 0b11);
 *
 * Will set the two dots under P and L
 */
var mod = module.exports;

var DECODE_MODE = 0x9;
var INTENSITY = 0xA;
var SCAN_LIMIT = 0xB;
var SHUTDOWN = 0xC;
var DIGIT_MAP = "0123456789-EHLP ";

mod.instance = function(spi_port, cs)
{
    this.spi_port = spi_port;
    this.cs = cs;
    this.send(INTENSITY, 0xF);
    this.send(SCAN_LIMIT, 0x7);
    this.send(DECODE_MODE, 0xFF); /* Code B */
    this.send(SHUTDOWN, 0x0);
};

mod.instance.prototype.send = function(reg, data)
{
    this.spi_port.send([reg, data], this.cs);
};

mod.instance.prototype.set = function(str, dot_map)
{
    var len = str.length - 1;
    var blank = DIGIT_MAP.indexOf(" ");

    if (len > 7)
        len = 7;

    for (var i = 0; i < 8; i++)
    {
        var idx = blank;

        if (i <= len)
        {
            var cidx = DIGIT_MAP.indexOf(str[len - i]);

            if (cidx != -1)
                idx = cidx;
        }
        if (dot_map && dot_map & (1<<i))
            idx |= 0x80;
        this.send(i + 1, idx);
    }
    this.send(SHUTDOWN, 1);
};
