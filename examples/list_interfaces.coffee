wl = require('../client')

display = wl.connect()

registry = display.get_registry()
registry.listen {
    global: (id, name, version) ->
        console.log id, name, version
}

display.roundtrip()
display.disconnect()
