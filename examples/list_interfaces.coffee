wl = require('../client')

display = wl.connect()

registry = display.get_registry()
registry.listen {
    global: (id, name, version) ->
        console.log 'global', id, name, version
}

display.roundtrip()
display.disconnect()
