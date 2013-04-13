wl = require '../client'
fs = require 'fs'

display = wl.connect()
use = {
    wl_compositor: ['compositor', 1],
    wl_shell:      ['shell',      1],
    wl_shm:        ['shm',        1],
}
registry = display.get_registry()
registry.listen {
    global: (id, name, version) ->
        target = use[name]
        if target
            [key, version] = target
            display[key] = registry.bind(id, wl.interfaces[name], version)
}
display.roundtrip()

display.shm.formats = 0
display.shm.listen {
    format: (format) ->
        display.shm.formats |= 1 << format
}
display.roundtrip()
unless display.shm.formats & (1 << wl.FORMAT_XRGB8888)
    throw "SHM_FORMAT_XRGB8888 not available"

window = {}
window.surface = display.compositor.create_surface()
window.shell_surface = display.shell.get_shell_surface(window.surface)
window.shell_surface.listen {
    ping: (serial) -> window.shell_surface.pong(serial)
}
window.shell_surface.set_title("simple-shm")
window.shell_surface.set_toplevel()

info = {}
info.width  = 250
info.height = 250
info.format = display.shm.FORMAT_XRGB8888
info.stride = info.width*4
info.size   = info.stride*info.height
info.fd     = fs.openSync('/dev/zero', 'r+')
# to make this demo show something interesting
# node.js needs anonymous files and mmap of them into array buffers.

pool = display.shm.create_pool(info.fd, info.size)
buffer = pool.create_buffer(0, info.width, info.height, info.stride, info.format)
info.busy   = false
buffer.listen {
    release: () -> info.busy = false
}
pool.destroy()
fs.close(info.fd)

window.surface.attach buffer, 0, 0
window.surface.damage 0, 0, info.width, info.height
window.surface.commit()
info.busy = true

ret = 0
while ret != -1
    ret = display.dispatch()
console.log ret
