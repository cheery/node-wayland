var wl = require('./build/Release/wayland_client');
var pr = require('./client-protocol')

pr.Display.prototype.roundtrip = function() {
    wl.display_roundtrip(this.proxy);
};

pr.Display.prototype.disconnect = function() {
    wl.display_disconnect(this.proxy);
};

pr.Display.prototype.fileno = function() {
    wl.display_fileno(this.proxy);
};

pr.Display.prototype.flush = function() {
    wl.display_flush(this.proxy);
};

exports.connect = function(name) {
    return new pr.Display(wl.connect(name));
}

exports.connect_to_fd = function(fd) {
    return new pr.Display(wl.connect_to_fd(fd));
}

exports.get_interface_by_name = wl.get_interface_by_name
