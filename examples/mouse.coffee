wl = require '../client'
fs = require 'fs'

display = wl.connect()

use = {
    wl_compositor: ['compositor', 1],
    wl_shell:      ['shell',      1],
    wl_shm:        ['shm',        1],
    wl_seat:       ['seat',       1],
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

display.seat.capabilities = 0
display.seat.listen {
    capabilities: (flags) -> this.capabilities = flags
}
display.roundtrip()
unless display.shm.formats & (1 << wl.FORMAT_XRGB8888)
    throw "SHM_FORMAT_XRGB8888 not available"
unless display.seat.capabilities & display.seat.CAPABILITY_POINTER
    throw "seat has no mouse"

window = {}
window.surface = display.compositor.create_surface()
window.shell_surface = display.shell.get_shell_surface(window.surface)
window.shell_surface.listen {
    ping: (serial) -> window.shell_surface.pong(serial)
}
window.shell_surface.set_title("simple-shm")
window.shell_surface.set_toplevel()

info = {}
info.width  = 256
info.height = 256
info.format = display.shm.FORMAT_XRGB8888
info.stride = info.width*4
info.size   = info.stride*info.height
info.fd     = wl.create_anonymous_file()
fs.truncateSync(info.fd, info.size)
data = wl.mmap_fd(info.fd, info.size)

for i in [0...info.width*info.height]
    x = i % info.width
    y = Math.floor(i / info.width)
    value = 0xA0
    x_even = Math.floor(x/16) % 2 == 0
    y_even = Math.floor(y/16) % 2 == 0
    if x_even ^ y_even
        value = 0x40
    data[i*4+3] = value
    data[i*4+2] = value
    data[i*4+1] = value
    data[i*4+0] = value

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

display.roundtrip()

console.log "window surface id #{window.surface.proxy.get_id()}"

# the enter and leave events return an ID to proxy, though
# I'm not sure which way to retrieve it yet.
# so I just drop out an id.

# interface requests a cursor image at enter. I give it nothing
# so the cursor disappears when it goes into my window.

# the seat seems to not allow me to grab it for the program.

pointer = display.seat.get_pointer()
pointer.listen {
    motion: (time, x, y) ->
        console.log "motion #{time} (#{x},#{y})"
    button: (serial, time, button, state) ->
        console.log "button #{serial} #{time} #{button} #{state}"
        if state == 1 and this.inside == window.surface.proxy.get_id()
            window.shell_surface.move(display.seat, serial)
    axis: (time, axis, value) ->
        console.log "button #{time} #{axis} #{value}"
    enter: (serial, which, x, y) ->
        console.log "enter #{serial} #{which} (#{x},#{y})"
        this.set_cursor(serial, null, 0, 0)
        this.inside = which
    leave: (serial, which) ->
        console.log "leave #{serial} #{which}"
        this.inside = null
}

ret = 0
while ret != -1
    ret = display.dispatch()
console.log ret
