/* Simple driver for Hitach HD44780 like text LCD screens */

TextLCD = function(rs,en,d4,d5,d6,d7) {
    this.data = [d7,d6,d5,d4];
    this.rs = rs;
    this.en = en;

    digitalWrite(this.en, 0);

    /* Function Set x2: these are special cases instructing a reset
     */
    this.cmd(0x33);
    /* Function Set x2:
     * 1. Special case instructing reset
     * 2. Instruct the LCD to move to 4 bit mode
     */
    this.cmd(0x32);
    /* Function Set: Data length 4 bit 2 lines, 5x7 Font */
    this.cmd(0x28);
    /* Display On/Off: Display Off, blinking Off */
    this.cmd(0x08);
    /* Clear the screen */
    this.clear(0x01);
    /* Entry Mode Set: Enable cursor shift, no autoscroll */
    this.cmd(0x06);
    /* Display On/Off: Display On, blinking Off */
    this.cmd(0x0C);
};

TextLCD.prototype.xmit = function(d) {
    digitalWrite(this.data, d>>4);
    digitalPulse(this.en, 1, 0.01);
    digitalWrite(this.data, d);
    digitalPulse(this.en, 1, 0.01);
};

TextLCD.prototype.cmd = function(c) {
    digitalWrite(this.rs, 0);
    this.xmit(c);
};

TextLCD.prototype.clear = function() { this.cmd(0x01); };
TextLCD.prototype.autoScroll = function() { this.cmd(0x07); };
TextLCD.prototype.noAutoScroll = function() { this.cmd(0x06); };

TextLCD.prototype.print = function(s) {
    digitalWrite(this.rs, 1);
    for (var i=0;i<s.length;i++)
	this.xmit(s.charCodeAt(i));
};

/* Set cursor position, top left = 0,0 */
TextLCD.prototype.setCursor = function(x,y) { 
    var l=[0x00,0x40,0x14,0x54];
    this.cmd(0x80|(l[y]+x));
};
