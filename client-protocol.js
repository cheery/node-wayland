// do not extend this file, rather.. make a generator to generate it.
var wl = require('./build/Release/wayland_client');

function Display(proxy) {
    proxy.spy(this);
    this.proxy = proxy;
}

WL_DISPLAY_GET_REGISTRY = 1

Display.prototype = {
    get_registry: function() {
        registry = this.proxy.create(wl.get_interface_by_name('wl_registry'))
        this.proxy.marshal(WL_DISPLAY_GET_REGISTRY, registry);
        return new Registry(registry);
    },
};

function Registry(proxy) {
    proxy.spy(this);
    this.proxy = proxy;
}

Registry.prototype = {
    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, arguments);
        });
    }
};

exports.Display = Display;
exports.Registry = Registry;

spies = {
    wl_display: Display,
    wl_registry: Registry
}
