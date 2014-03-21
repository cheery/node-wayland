wl = require '../client'
util = require './util'

display = wl.connect()
registry = display.get_registry()

globals = util.get_globals(registry)
display.roundtrip()

compositor = globals.bind_one('wl_compositor', 1)
shell = globals.bind_one('wl_shell', 1)
shm = globals.bind_one('wl_shm', 1)
seat = globals.bind_one('wl_seat', 1)
data_device_manager = globals.bind_one('wl_data_device_manager', 1)

shm.formats = 0
shm.listen format: (format) -> @formats |= 1 << format
seat.capabilities = 0
seat.listen capabilities: (flags) -> @capabilities = flags
display.roundtrip()
unless shm.formats & (1 << wl.FORMAT_XRGB8888)
    throw "SHM_FORMAT_XRGB8888 not available"
unless seat.capabilities & seat.CAPABILITY_POINTER
    throw "seat has no mouse"
unless seat.capabilities & seat.CAPABILITY_KEYBOARD
    throw "seat has no keyboard"

cursor = util.create_buffer(shm, 32, 32)
cursor.surface = compositor.create_surface()
update_cursor = () ->
    for i in [0...cursor.width*cursor.height]
        x = (i % cursor.width)
        y = Math.floor(i / cursor.width)
        x_even = Math.floor(x/4) % 2 == 0
        y_even = Math.floor(y/4) % 2 == 0
        value = 0xff
        if x_even ^ y_even
            value = 0x40
        cursor.data[i*4+3] = value
        cursor.data[i*4+2] = value
        cursor.data[i*4+1] = value
        cursor.data[i*4+0] = value

    cursor.surface.attach(cursor.buffer, 0, 0)
    cursor.surface.damage(0, 0, cursor.width, cursor.height)
    cursor.surface.commit()
    cursor.busy = true


window = util.create_widget(compositor, shell, shm, 256, 256)
window.shellface.set_toplevel()
window.fullscreen = false

redraw = (time) ->
    {width, height, data, busy} = window.buffers.back
    if busy then throw "buffers busy, can't draw"

    offset = Math.sin(time)

    for i in [0...width*height]
        x = (i % width)
        y = Math.floor(i / width)
        value = 0x80 + offset * 0x80
        x_even = Math.floor(x/16) % 2 == 0
        y_even = Math.floor(y/16) % 2 == 0
        if x_even ^ y_even
            value = 0x40
        data[i*4+3] = value
        data[i*4+2] = value
        data[i*4+1] = value
        data[i*4+0] = value

    window.frame(redraw)
    window.flip()

redraw(0)

keyboard = seat.get_keyboard()
keyboard.listen {
    enter: (serial, surface, keys) ->
        console.log "keyboard enter", serial, surface, keys
    leave: (serial, surface) ->
        console.log "keyboard leave", serial, surface
    key: (serial, time, key, state) ->
        console.log "key", serial, time, key, state
        if key == 87 and state == 1
            if window.fullscreen
                window.fullscreen = false
                window.shellface.set_toplevel()
            else
                window.fullscreen = true
                window.shellface.set_fullscreen(
                    window.shellface.FULLSCREEN_METHOD_SCALE,
                    30,
                    null
                )
}

pointer = seat.get_pointer()
pointer.inside = null
pointer.listen {
    motion: (time, x, y) ->
#        console.log "motion #{time} (#{x},#{y})"
    button: (serial, time, button, state) ->
        console.log "button #{serial} #{time} #{button} #{state}"
        console.log 'this inside', this.inside
        console.log 'window surface', window.surface
        if state == 1 and this.inside == window.surface
            window.shellface.move(seat, serial)
    axis: (time, axis, value) ->
#        console.log "button #{time} #{axis} #{value}"
    enter: (serial, which, x, y) ->
        console.log "enter #{serial} #{which} (#{x},#{y})"
        this.set_cursor(serial, cursor.surface, 32, 32)
        update_cursor()
        this.inside = which
    leave: (serial, which) ->
#        console.log "leave #{serial} #{which}"
        this.inside = null
}

# create_data_source
data_device = data_device_manager.get_data_device(seat)
data_device.proxy.listen (args...) ->
    console.log args

ret = 0
while ret != -1
    ret = display.dispatch()
console.log ret
