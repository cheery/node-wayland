#include <v8.h>
#include <node.h>
#include <node_buffer.h>

using namespace v8;

extern "C" {
    #include <wayland-client.h>
    #include <stdio.h>
    #include <string.h>
}

#include "interface.h"

Persistent<Function> Interface::constructor;

void Interface::Init(Handle<Object> target) {
    Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    tpl->SetClassName(String::NewSymbol("Interface"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    NODE_SET_PROTOTYPE_METHOD(tpl, "get_name", GetName);
    constructor = Persistent<Function>::New(tpl->GetFunction());
    target->Set(String::NewSymbol("Interface"), constructor);
}

Handle<Value> Interface::New(const Arguments& args) {
    HandleScope scope;
    Interface* interface = new Interface();
    interface->interface = (const struct wl_interface*)External::Unwrap(args[0]);
    interface->Wrap(args.This());
    return args.This();
};
Handle<Value> Interface::GetName(const Arguments& args) {
    HandleScope scope;
    const struct wl_interface* interface = Unwrap(args.This());
    return scope.Close(String::New(interface->name));
}

static const struct wl_interface* interfaces[] = {
    &wl_display_interface,
    &wl_registry_interface,
    &wl_callback_interface,
    &wl_compositor_interface,
    &wl_shm_pool_interface,
    &wl_shm_interface,
    &wl_buffer_interface,
    &wl_data_offer_interface,
    &wl_data_source_interface,
    &wl_data_device_interface,
    &wl_data_device_manager_interface,
    &wl_shell_interface,
    &wl_shell_surface_interface,
    &wl_surface_interface,
    &wl_seat_interface,
    &wl_pointer_interface,
    &wl_keyboard_interface,
    &wl_touch_interface,
    &wl_output_interface,
    &wl_region_interface,
    NULL,
};

static const struct wl_interface* get_interface_by_name(const char* name) {
    for (int i = 0; interfaces[i] != NULL; i++) {
        if (strcmp(interfaces[i]->name, name) == 0) { return interfaces[i]; }
    }
    return NULL;
}

Handle<Value> Interface::GetInterfaceByName(const Arguments& args) {
    HandleScope scope;
    v8::String::AsciiValue string(args[0]);
    const struct wl_interface* interface = get_interface_by_name(*string);
    if (interface == NULL) return scope.Close(Undefined());
    const unsigned argc = 1;
    Handle<Value> argv[argc] = { External::Wrap((void*)interface) };
    Local<Object> instance = Interface::constructor->NewInstance(argc, argv);
    return scope.Close(instance);
}
