wl = require '../client'
fs = require 'fs'

exports.get_globals = (registry) ->
    out = {}
    registry.listen global: (id, name, version) ->
        out[name] ?= []
        out[name].push {id, version}
    
    return {
        bind_one: (name, version) ->
            unless out[name] and out[name].length > 0
                return null
            {id, newest} = out[name].pop()
            version ?= newest
            return registry.bind(id, wl.interfaces[name], version)
    }

exports.create_buffer = (shm, width, height) ->
    format = shm.FORMAT_XRGB8888
    stride = width*4
    size   = stride*height
    fd     = wl.create_anonymous_file()
    fs.truncate(fd, size)
    data = wl.mmap_fd(fd, size)
    pool = shm.create_pool(fd, size)
    buffer = pool.create_buffer(0, width, height, stride, format)
    obj = {width, height, format, stride, size, data, buffer, busy:false}
    buffer.listen release: () -> obj.busy = false
    pool.destroy()
    fs.close(fd)
    return obj

exports.create_surface

exports.create_widget = (compositor, shell, shm, width, height) ->
    surface = compositor.create_surface()
    shellface = shell.get_shell_surface(surface)
    shellface.listen
        ping: (serial) -> shellface.pong(serial)

    front = exports.create_buffer(shm, width, height)
    back  = exports.create_buffer(shm, width, height)

    return {
        surface,
        shellface,
        width,
        height,
        buffers: {front, back}
        flip: () ->
            {front, back} = @buffers
            surface.attach(back.buffer, 0, 0)
            surface.damage(0, 0, @width, @height)
            surface.commit()
            back.busy = true
            @buffers = front:back, back:front
        frame: (fn) ->
            if @callback then throw "frame already requested"
            @callback = @surface.frame()
            @callback.listen done: (time) =>
                @callback.destroy()
                @callback = undefined
                fn.call(@, time / 1000)
    }
