var wl = require('./build/Release/wayland_client');
var interfaces = {};
exports.interfaces = interfaces;
/*
    Copyright © 2008-2011 Kristian Høgsberg
    Copyright © 2010-2011 Intel Corporation

    Permission to use, copy, modify, distribute, and sell this
    software and its documentation for any purpose is hereby granted
    without fee, provided that the above copyright notice appear in
    all copies and that both that copyright notice and this permission
    notice appear in supporting documentation, and that the name of
    the copyright holders not be used in advertising or publicity
    pertaining to distribution of the software without specific,
    written prior permission.  The copyright holders make no
    representations about the suitability of this software for any
    purpose.  It is provided "as is" without express or implied
    warranty.

    THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
    SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
    FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
    SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
    AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
    ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
    THIS SOFTWARE.
*/
function wl_display(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
wl_display.prototype = {
    sync: function() {
        new_id = this.proxy.create(wl_callback.interface);
        this.proxy.marshal(0, new_id);
        return new wl_callback(new_id);
    },
    get_registry: function() {
        new_id = this.proxy.create(wl_registry.interface);
        this.proxy.marshal(1, new_id);
        return new wl_registry(new_id);
    },
ERROR_INVALID_OBJECT: 0,
ERROR_INVALID_METHOD: 1,
ERROR_NO_MEMORY: 2
};
wl_display.interface = wl.get_interface_by_name('wl_display');
interfaces['wl_display'] = wl_display

function wl_registry(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
wl_registry.prototype = {
    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, Array.prototype.slice.call(arguments, 1));
        });
    },
    bind: function(name, spy, version) {
        new_id = this.proxy.create(spy.interface);
        this.proxy.marshal(0, name, spy.interface.get_name(), version, new_id);
        return new spy(new_id);
    }
};
wl_registry.interface = wl.get_interface_by_name('wl_registry');
interfaces['wl_registry'] = wl_registry

function wl_callback(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
wl_callback.prototype = {
    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, Array.prototype.slice.call(arguments, 1));
        });
    }
};
wl_callback.interface = wl.get_interface_by_name('wl_callback');
interfaces['wl_callback'] = wl_callback

function wl_compositor(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
wl_compositor.prototype = {
    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, Array.prototype.slice.call(arguments, 1));
        });
    },
    create_surface: function() {
        new_id = this.proxy.create(wl_surface.interface);
        this.proxy.marshal(0, new_id);
        return new wl_surface(new_id);
    },
    create_region: function() {
        new_id = this.proxy.create(wl_region.interface);
        this.proxy.marshal(1, new_id);
        return new wl_region(new_id);
    }
};
wl_compositor.interface = wl.get_interface_by_name('wl_compositor');
interfaces['wl_compositor'] = wl_compositor

function wl_shm_pool(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
wl_shm_pool.prototype = {
    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, Array.prototype.slice.call(arguments, 1));
        });
    },
    create_buffer: function(offset, width, height, stride, format) {
        new_id = this.proxy.create(wl_buffer.interface);
        this.proxy.marshal(0, new_id, offset, width, height, stride, format);
        return new wl_buffer(new_id);
    },
    destroy: function() {
        this.proxy.marshal(1);
    },
    resize: function(size) {
        this.proxy.marshal(2, size);
    }
};
wl_shm_pool.interface = wl.get_interface_by_name('wl_shm_pool');
interfaces['wl_shm_pool'] = wl_shm_pool

function wl_shm(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
wl_shm.prototype = {
    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, Array.prototype.slice.call(arguments, 1));
        });
    },
    create_pool: function(fd, size) {
        new_id = this.proxy.create(wl_shm_pool.interface);
        this.proxy.marshal(0, new_id, fd, size);
        return new wl_shm_pool(new_id);
    },
ERROR_INVALID_FORMAT: 0,
ERROR_INVALID_STRIDE: 1,
ERROR_INVALID_FD: 2,
FORMAT_ARGB8888: 0,
FORMAT_XRGB8888: 1
};
wl_shm.interface = wl.get_interface_by_name('wl_shm');
interfaces['wl_shm'] = wl_shm

function wl_buffer(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
wl_buffer.prototype = {
    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, Array.prototype.slice.call(arguments, 1));
        });
    },
    destroy: function() {
        this.proxy.marshal(0);
    }
};
wl_buffer.interface = wl.get_interface_by_name('wl_buffer');
interfaces['wl_buffer'] = wl_buffer

function wl_data_offer(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
wl_data_offer.prototype = {
    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, Array.prototype.slice.call(arguments, 1));
        });
    },
    accept: function(serial, mime_type) {
        this.proxy.marshal(0, serial, mime_type);
    },
    receive: function(mime_type, fd) {
        this.proxy.marshal(1, mime_type, fd);
    },
    destroy: function() {
        this.proxy.marshal(2);
    }
};
wl_data_offer.interface = wl.get_interface_by_name('wl_data_offer');
interfaces['wl_data_offer'] = wl_data_offer

function wl_data_source(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
wl_data_source.prototype = {
    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, Array.prototype.slice.call(arguments, 1));
        });
    },
    offer: function(mime_type) {
        this.proxy.marshal(0, mime_type);
    },
    destroy: function() {
        this.proxy.marshal(1);
    }
};
wl_data_source.interface = wl.get_interface_by_name('wl_data_source');
interfaces['wl_data_source'] = wl_data_source

function wl_data_device(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
wl_data_device.prototype = {
    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, Array.prototype.slice.call(arguments, 1));
        });
    },
    start_drag: function(source, origin, icon, serial) {
        this.proxy.marshal(0, (source === null || source === undefined)?source:source.proxy, (origin === null || origin === undefined)?origin:origin.proxy, (icon === null || icon === undefined)?icon:icon.proxy, serial);
    },
    set_selection: function(source, serial) {
        this.proxy.marshal(1, (source === null || source === undefined)?source:source.proxy, serial);
    }
};
wl_data_device.interface = wl.get_interface_by_name('wl_data_device');
interfaces['wl_data_device'] = wl_data_device

function wl_data_device_manager(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
wl_data_device_manager.prototype = {
    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, Array.prototype.slice.call(arguments, 1));
        });
    },
    create_data_source: function() {
        new_id = this.proxy.create(wl_data_source.interface);
        this.proxy.marshal(0, new_id);
        return new wl_data_source(new_id);
    },
    get_data_device: function(seat) {
        new_id = this.proxy.create(wl_data_device.interface);
        this.proxy.marshal(1, new_id, (seat === null || seat === undefined)?seat:seat.proxy);
        return new wl_data_device(new_id);
    }
};
wl_data_device_manager.interface = wl.get_interface_by_name('wl_data_device_manager');
interfaces['wl_data_device_manager'] = wl_data_device_manager

function wl_shell(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
wl_shell.prototype = {
    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, Array.prototype.slice.call(arguments, 1));
        });
    },
    get_shell_surface: function(surface) {
        new_id = this.proxy.create(wl_shell_surface.interface);
        this.proxy.marshal(0, new_id, (surface === null || surface === undefined)?surface:surface.proxy);
        return new wl_shell_surface(new_id);
    }
};
wl_shell.interface = wl.get_interface_by_name('wl_shell');
interfaces['wl_shell'] = wl_shell

function wl_shell_surface(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
wl_shell_surface.prototype = {
    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, Array.prototype.slice.call(arguments, 1));
        });
    },
    pong: function(serial) {
        this.proxy.marshal(0, serial);
    },
    move: function(seat, serial) {
        this.proxy.marshal(1, (seat === null || seat === undefined)?seat:seat.proxy, serial);
    },
    resize: function(seat, serial, edges) {
        this.proxy.marshal(2, (seat === null || seat === undefined)?seat:seat.proxy, serial, edges);
    },
    set_toplevel: function() {
        this.proxy.marshal(3);
    },
    set_transient: function(parent, x, y, flags) {
        this.proxy.marshal(4, (parent === null || parent === undefined)?parent:parent.proxy, x, y, flags);
    },
    set_fullscreen: function(method, framerate, output) {
        this.proxy.marshal(5, method, framerate, (output === null || output === undefined)?output:output.proxy);
    },
    set_popup: function(seat, serial, parent, x, y, flags) {
        this.proxy.marshal(6, (seat === null || seat === undefined)?seat:seat.proxy, serial, (parent === null || parent === undefined)?parent:parent.proxy, x, y, flags);
    },
    set_maximized: function(output) {
        this.proxy.marshal(7, (output === null || output === undefined)?output:output.proxy);
    },
    set_title: function(title) {
        this.proxy.marshal(8, title);
    },
    set_class: function(class_) {
        this.proxy.marshal(9, class_);
    },
RESIZE_NONE: 0,
RESIZE_TOP: 1,
RESIZE_BOTTOM: 2,
RESIZE_LEFT: 4,
RESIZE_TOP_LEFT: 5,
RESIZE_BOTTOM_LEFT: 6,
RESIZE_RIGHT: 8,
RESIZE_TOP_RIGHT: 9,
RESIZE_BOTTOM_RIGHT: 10,
TRANSIENT_INACTIVE: 0x1,
FULLSCREEN_METHOD_DEFAULT: 0,
FULLSCREEN_METHOD_SCALE: 1,
FULLSCREEN_METHOD_DRIVER: 2,
FULLSCREEN_METHOD_FILL: 3
};
wl_shell_surface.interface = wl.get_interface_by_name('wl_shell_surface');
interfaces['wl_shell_surface'] = wl_shell_surface

function wl_surface(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
wl_surface.prototype = {
    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, Array.prototype.slice.call(arguments, 1));
        });
    },
    destroy: function() {
        this.proxy.marshal(0);
    },
    attach: function(buffer, x, y) {
        this.proxy.marshal(1, (buffer === null || buffer === undefined)?buffer:buffer.proxy, x, y);
    },
    damage: function(x, y, width, height) {
        this.proxy.marshal(2, x, y, width, height);
    },
    frame: function() {
        new_id = this.proxy.create(wl_callback.interface);
        this.proxy.marshal(3, new_id);
        return new wl_callback(new_id);
    },
    set_opaque_region: function(region) {
        this.proxy.marshal(4, (region === null || region === undefined)?region:region.proxy);
    },
    set_input_region: function(region) {
        this.proxy.marshal(5, (region === null || region === undefined)?region:region.proxy);
    },
    commit: function() {
        this.proxy.marshal(6);
    },
    set_buffer_transform: function(transform) {
        this.proxy.marshal(7, transform);
    }
};
wl_surface.interface = wl.get_interface_by_name('wl_surface');
interfaces['wl_surface'] = wl_surface

function wl_seat(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
wl_seat.prototype = {
    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, Array.prototype.slice.call(arguments, 1));
        });
    },
    get_pointer: function() {
        new_id = this.proxy.create(wl_pointer.interface);
        this.proxy.marshal(0, new_id);
        return new wl_pointer(new_id);
    },
    get_keyboard: function() {
        new_id = this.proxy.create(wl_keyboard.interface);
        this.proxy.marshal(1, new_id);
        return new wl_keyboard(new_id);
    },
    get_touch: function() {
        new_id = this.proxy.create(wl_touch.interface);
        this.proxy.marshal(2, new_id);
        return new wl_touch(new_id);
    },
CAPABILITY_POINTER: 1,
CAPABILITY_KEYBOARD: 2,
CAPABILITY_TOUCH: 4
};
wl_seat.interface = wl.get_interface_by_name('wl_seat');
interfaces['wl_seat'] = wl_seat

function wl_pointer(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
wl_pointer.prototype = {
    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, Array.prototype.slice.call(arguments, 1));
        });
    },
    set_cursor: function(serial, surface, hotspot_x, hotspot_y) {
        this.proxy.marshal(0, serial, (surface === null || surface === undefined)?surface:surface.proxy, hotspot_x, hotspot_y);
    },
BUTTON_STATE_RELEASED: 0,
BUTTON_STATE_PRESSED: 1,
AXIS_VERTICAL_SCROLL: 0,
AXIS_HORIZONTAL_SCROLL: 1
};
wl_pointer.interface = wl.get_interface_by_name('wl_pointer');
interfaces['wl_pointer'] = wl_pointer

function wl_keyboard(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
wl_keyboard.prototype = {
    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, Array.prototype.slice.call(arguments, 1));
        });
    },
KEYMAP_FORMAT_XKB_V1: 1,
KEY_STATE_RELEASED: 0,
KEY_STATE_PRESSED: 1
};
wl_keyboard.interface = wl.get_interface_by_name('wl_keyboard');
interfaces['wl_keyboard'] = wl_keyboard

function wl_touch(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
wl_touch.prototype = {
    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, Array.prototype.slice.call(arguments, 1));
        });
    }
};
wl_touch.interface = wl.get_interface_by_name('wl_touch');
interfaces['wl_touch'] = wl_touch

function wl_output(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
wl_output.prototype = {
    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, Array.prototype.slice.call(arguments, 1));
        });
    },
SUBPIXEL_UNKNOWN: 0,
SUBPIXEL_NONE: 1,
SUBPIXEL_HORIZONTAL_RGB: 2,
SUBPIXEL_HORIZONTAL_BGR: 3,
SUBPIXEL_VERTICAL_RGB: 4,
SUBPIXEL_VERTICAL_BGR: 5,
TRANSFORM_NORMAL: 0,
TRANSFORM_90: 1,
TRANSFORM_180: 2,
TRANSFORM_270: 3,
TRANSFORM_FLIPPED: 4,
TRANSFORM_FLIPPED_90: 5,
TRANSFORM_FLIPPED_180: 6,
TRANSFORM_FLIPPED_270: 7,
MODE_CURRENT: 0x1,
MODE_PREFERRED: 0x2
};
wl_output.interface = wl.get_interface_by_name('wl_output');
interfaces['wl_output'] = wl_output

function wl_region(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
wl_region.prototype = {
    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, Array.prototype.slice.call(arguments, 1));
        });
    },
    destroy: function() {
        this.proxy.marshal(0);
    },
    add: function(x, y, width, height) {
        this.proxy.marshal(1, x, y, width, height);
    },
    subtract: function(x, y, width, height) {
        this.proxy.marshal(2, x, y, width, height);
    }
};
wl_region.interface = wl.get_interface_by_name('wl_region');
interfaces['wl_region'] = wl_region

