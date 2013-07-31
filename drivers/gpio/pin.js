Pin = function(p) {
    var self = this;

    self.on = false;
    self.pin_num = p;
    self.set = function() { digitalWrite(self.pin_num, 1); };
    self.clear = function() { digitalWrite(self.pin_num, 0); };
    self.write = function(v) { digitalWrite(self.pin_num, v); };
    self.read = function() { return digitalRead(self.pin_num); };
    self.toggle = function() { self.on=!self.on; self.write(self.on); };
    self.clear();
};
