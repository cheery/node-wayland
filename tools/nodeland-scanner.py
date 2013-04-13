from lxml import etree
from sys import stdout, stderr

def mk_comment(text):
    return '/*'+text.strip(' ')+'*/\n'

header_gen = u"""var wl = require('./build/Release/wayland_client');
var interfaces = {};
exports.interfaces = interfaces;
"""
interface_template = u"""function %(name)s(proxy) {
    this.proxy = proxy;
    proxy.spy(this);
};
%(name)s.prototype = {
%(prototype)s
};
%(name)s.interface = wl.get_interface_by_name(%(name)r);
interfaces[%(name)r] = %(name)s

"""
default_proxy_template = """    listen: function(listeners) {
        var self = this;
        this.proxy.listen(function(name){
            if (listeners[name]) listeners[name].apply(self, Array.prototype.slice.call(arguments, 1));
        });
    },
    destroy: function() {
        this.proxy.destroy()
    }"""
request_template = u"""    %(name)s: function(%(args)s) {
        this.proxy.marshal(%(argv)s);
    }"""
factory_template = u"""    %(name)s: function(%(args)s) {
        new_id = this.proxy.create(%(spy)s.interface);
        this.proxy.marshal(%(argv)s);
        return new %(spy)s(new_id);
    }"""
factory_dyn_template = u"""    %(name)s: function(%(args)s) {
        new_id = this.proxy.create(spy.interface);
        this.proxy.marshal(%(argv)s);
        return new spy(new_id);
    }"""

def generate_request(index, request):
    data = dict(name = request.attrib['name'], magic=index)
    args = []
    argv = [str(index)]
    template = request_template
    for node in request:
        if node.tag == 'arg' and node.attrib['type'] in ('int', 'uint', 'fd', 'string', 'fixed'):
            name = node.attrib['name']
            args.append(name)
            argv.append(name)
        elif node.tag == 'arg' and node.attrib['type'] == 'object':
            name = node.attrib['name']
            args.append(name)
            argv.append('(%(var)s === null || %(var)s === undefined)?%(var)s:%(var)s.proxy' % dict(var=name))
        elif node.tag == 'arg' and node.attrib['type'] == 'new_id':
            if 'interface' in node.attrib:
                template = factory_template
                data['spy'] = node.attrib['interface']
                argv.append('new_id')
            else:
                template = factory_dyn_template
                args.append('spy, version')
                argv.append('spy.interface.get_name(), version, new_id')
        elif node.tag == 'description':
            continue
        else:
            stderr.write("%s %r %r" % (node.tag, node.attrib, node[:]))
            stderr.write("\n")
            raise Exception("unknown argument node %r" % node)
    data['args'] = ', '.join(args)
    data['argv'] = ', '.join(argv)
    return template % data

def generate_enum_const(enum_name, const):
    data = dict(
        name=('%s_%s' % (enum_name, const.attrib['name'])).upper(),
        value=const.attrib['value'],
    )
    return '%(name)s: %(value)s' % data

def generate_interface(interface):
    count = 0
    methods = []
    enums = []
    name = interface.attrib['name']
    if name != 'wl_display':
        methods.append(default_proxy_template)
    for node in interface:
        if node.tag == 'description':
            continue
        elif node.tag == 'request':
            methods.append(generate_request(count, node))
            count += 1
        elif node.tag == 'event':
            continue
        elif node.tag == 'enum':
            enum_name = node.attrib['name']
            for node in node:
                if node.tag == 'entry':
                    enums.append(generate_enum_const(enum_name, node))
                elif node.tag == 'description':
                    continue
                else:
                    stderr.write("%s %r %r" % (node.tag, node.attrib, node[:]))
                    stderr.write("\n")
                    raise Exception("unknown entry node %r" % node)
        elif node.tag == etree.Comment:
            continue
        else:
            raise Exception("unknown interface node %r" % node)
    return dict(prototype=',\n'.join(methods + enums))

root = etree.parse("wayland.xml").getroot()
stdout.write(header_gen)
for node in root:
    if node.tag == 'copyright':
        stdout.write(mk_comment(node.text).encode('utf-8'))
    elif node.tag == 'interface':
        data = generate_interface(node)
        data.update(node.attrib)
        stdout.write((interface_template % data).encode('utf-8'))
    else:
        raise Exception("unknown root node")
