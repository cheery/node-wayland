# This program is a mockup, this program does not run yet.
wl = require 'wayland/client'

## Display
display = wl.connect()
registry = display.get_registry()
registry.listen {
    global: (id, interface, version) ->
        if interface == 'wl_compositor'
            display.compositor = registry.bind(id, get_interface_by_name(interface), 1)
        if interface == 'wl_shell'
            display.shell = registry.bind(id, get_interface_by_name(interface), 1)
        if interface == 'wl_shm'
            display.shm = registry.bind(id, get_interface_by_name(interface), 1)
}
display.roundtrip()
unless g.shm
    throw "no wl_shm global"
display->formats = 0
display.shm.listen {
    format: (format) ->
        display->formats = 1 << format
}
display.roundtrip()
unless display->formats & (1 << wl.SHM_FORMAT_XRGB8888)
    throw "SHM_FORMAT_XRGB8888 not available"

## Window
width = 250
height = 250
surface = display.compositor.create_surface()
shell_surface = display.shell.get_shell_surface(surface)

shell_surface.listen {
    ping: (serial) -> shell_surface.pong(serial)
}

shell_surface.set_title("simple-shm")
shell_surface.set_toplevel()

format = wl.SHM_FORMAT_XRGB8888
stride = width*4
size = stride*height

## Buffer
fd = wl.anonymous_file(size)
data = wl.mmap(size, 'rw', fd)
pool = display.shm.create_pool(fd, size)
buffer =  pool.create_buffer(0, width, height, stride, format)
buffer.busy = false
buffer.listen {
    release: () -> buffer.busy = false
}

## How to draw
fill(data)

surface.attach buffer, 0, 0
surface.damage 0, 0, width, height

surface.commit()
buffer.busy = true

## If you drew continuously, you'd do two buffers.
# it'd look approximately like this:

redraw = (time) ->
    # destroy the previous callback handle
    if surface.callback
        surface.callback.destroy()
    buffer = next_buffer()
    # buffer.busy must be 0
    fill(buffer.data)

    surface.attach buffer, 0, 0
    surface.damage 0, 0, width, height
    surface.commit()
    buffer.busy = 1

    surface.callback = surface.frame()
    surface.listen {redraw}

# drop into event loop
