var wl = require('./build/Release/wayland_client');
var interfaces = require('./client-protocol').interfaces;

wl_display = interfaces.wl_display
wl_display.prototype.roundtrip = function() {
    wl.display_roundtrip(this.proxy);
};

wl_display.prototype.disconnect = function() {
    wl.display_disconnect(this.proxy);
};

wl_display.prototype.fileno = function() {
    return wl.display_fileno(this.proxy);
};

wl_display.prototype.flush = function() {
    wl.display_flush(this.proxy);
};

wl_display.prototype.dispatch = function() {
    return wl.display_dispatch(this.proxy);
};

exports.connect = function(name) {
    return new wl_display(wl.connect(name));
};

exports.connect_to_fd = function(fd) {
    return new wl_display(wl.connect_to_fd(fd));
};
exports.interfaces = interfaces;

exports.create_anonymous_file = function() {
    return wl.create_anonymous_file();
};

exports.mmap_fd = function(fd, size) {
    data = wl.mmap_fd(fd, size);
    data.free = function() { wl.munmap_fd(this); }
    return data;
};

exports.munmap_fd = function(data) {
    wl.munmap_fd(data);
};
