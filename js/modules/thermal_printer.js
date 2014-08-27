/* Copyright (c) 2014 Eyal Birger, See LICENSE file for details.
 *
 * Mini Thermal Printer Driver module (e.g. https://learn.adafruit.com/mini-thermal-receipt-printer/)
 * Inspired by Adafruit-Thermal-Printer-Library (https://github.com/adafruit/Adafruit-Thermal-Printer-Library)
 * Connections: 1 VCC (5V), 4 TX, 5 GND
 * Usage:
 * var t = require('thermal_printer');
 * var p = new t.instance(new Serial(UART1, { baud_rate : 9600 }));
 * p.serial.write('Hello World!\n');
 */

var mod = module.exports;

mod.instance = function(serial_port)
{
    this.serial = serial_port;
    this.init();
};

mod.instance.prototype.setHeatTime = function(heatTime, heatInterval)
{
    this.serial.write(27);
    this.serial.write(55);
    this.serial.write(7);
    this.serial.write(heatTime);
    this.serial.write(heatInterval);
};

mod.instance.prototype.setPrintDensity = function(printDensity)
{
    this.serial.write([18, 35, (printDensity << 4) | 15]);
};

mod.instance.prototype.wake = function()
{
    this.serial.write([27, 61, 1]);
};

mod.instance.prototype.justify = function()
{
    this.serial.write([0x1b, 0x61, 0]);
};

mod.instance.prototype.inverseOff = function()
{
    this.serial.write([29, 'B', 0, 10]);
};

mod.instance.prototype.doubleHeightOff = function()
{
    this.serial.write([27, 20]);
};

mod.instance.prototype.setLineHeight = function(height)
{
    this.serial.write([27, 51, height]);
};

mod.instance.prototype.boldOff = function()
{
    this.serial.write([27, 69, 0]);
};

mod.instance.prototype.underlineOff = function()
{
    this.serial.write([27, 45, 0, 10]);
};

mod.instance.prototype.setSize = function()
{
    this.serial.write([29, 33, 0, 10]);
};

mod.instance.prototype.test = function()
{
    this.serial.print('kuku\n');
    this.serial.print('kuku!\n');
    this.serial.print('kuku!!\n');
};

mod.instance.prototype.init = function()
{
    this.serial.write([27, 64]);
    this.setHeatTime(80, 2);
    this.setPrintDensity(15);
    this.wake();
    this.justify();
    this.inverseOff();
    this.doubleHeightOff(32);
    this.setLineHeight(32);
    this.boldOff();
    this.underlineOff();
    this.setSize();
};
