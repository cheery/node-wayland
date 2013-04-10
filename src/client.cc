#include <v8.h>
#include <node.h>
#include <node_buffer.h>

using namespace v8;

extern "C" {
    #include <wayland-client.h>
    #include <stdio.h>
}

#include "interface.h"
#include "proxy.h"

//class Display : public node::ObjectWrap {
//    struct wl_display* display;
//    static inline Display* AsDisplay(Handle<Object> object) {
//        Display* display = ObjectWrap::Unwrap<Display>(object);
//        if (display->display == NULL) return NULL;
//        return display;
//    };
//public:
//    static void Init(Handle<Object> target) {
//        Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
//        tpl->SetClassName(String::NewSymbol("Display"));
//        tpl->InstanceTemplate()->SetInternalFieldCount(1);
//        NODE_SET_PROTOTYPE_METHOD(tpl, "close", Close);
//        NODE_SET_PROTOTYPE_METHOD(tpl, "fileno", Fileno);
//        NODE_SET_PROTOTYPE_METHOD(tpl, "flush", Flush);
//        NODE_SET_PROTOTYPE_METHOD(tpl, "roundtrip", Roundtrip);
//        //tpl->PrototypeTemplate()->Set( String::NewSymbol("close"), FunctionTemplate::New(Close)->GetFunction());
//        Persistent<Function> constructor = Persistent<Function>::New(tpl->GetFunction());
//        target->Set(String::NewSymbol("Display"), constructor);
//    };
//    static Handle<Value> New(const Arguments& args) {
//        HandleScope scope;
//        Display* display = new Display();
//        display->display = wl_display_connect(NULL);
//        if (display->display == NULL) return ThrowException(String::New("failed to connect"));
//        display->Wrap(args.This());
//        return args.This();
//    };
//
//};
static Handle<Value> Connect(const Arguments& args) {
    HandleScope scope;
    const char* name;
    if (args[0]->IsUndefined() || args[0]->IsNull()) {
        name = NULL;
    } else {
        v8::String::AsciiValue string(args[0]);
        name = *string;
    }
    wl_display* display = wl_display_connect(name);
    if (display == NULL) return ThrowException(String::New("failed to connect"));
    const unsigned argc = 2;
    Handle<Value> argv[argc] = {
        External::Wrap(display),
        External::Wrap((void*)&wl_display_interface),
    };
    Local<Object> instance = Proxy::constructor->NewInstance(argc, argv);
    return scope.Close(instance);
}

static Handle<Value> ConnectToFd(const Arguments& args) {
    HandleScope scope;
    wl_display* display = wl_display_connect_to_fd(args[0]->IntegerValue());
    if (display == NULL) return ThrowException(String::New("failed to connect"));
    const unsigned argc = 2;
    Handle<Value> argv[argc] = {
        External::Wrap(display),
        External::Wrap((void*)&wl_display_interface),
    };
    Local<Object> instance = Proxy::constructor->NewInstance(argc, argv);
    return scope.Close(instance);
}

static Handle<Value> DisplayClose(const Arguments& args) {
    HandleScope scope;
    Proxy* display = Proxy::AsProxy(args[0]->ToObject());
    if (display == NULL) return ThrowException(String::New("not connected"));
    wl_display_disconnect((wl_display*)display->proxy);
    display->Free();
    return scope.Close(Undefined());
};
static Handle<Value> DisplayFileno(const Arguments& args) {
    HandleScope scope;
    Proxy* display = Proxy::AsProxy(args[0]->ToObject());
    if (display == NULL) return ThrowException(String::New("not connected"));
    return scope.Close(Integer::New(wl_display_get_fd((wl_display*)display->proxy)));
};

static Handle<Value> DisplayFlush(const Arguments& args) {
    HandleScope scope;
    Proxy* display = Proxy::AsProxy(args[0]->ToObject());
    if (display == NULL) return ThrowException(String::New("not connected"));
    wl_display_flush((wl_display*)display->proxy);
    return scope.Close(Undefined());
};

static Handle<Value> DisplayRoundtrip(const Arguments& args) {
    HandleScope scope;
    Proxy* display = Proxy::AsProxy(args[0]->ToObject());
    if (display == NULL) return ThrowException(String::New("not connected"));
    wl_display_roundtrip((wl_display*)display->proxy);
    return scope.Close(Undefined());
};

static void Init(Handle<Object> target) {
    Interface::Init(target);
    Proxy::Init(target);
    target->Set(String::NewSymbol("connect"),
        FunctionTemplate::New(Connect)->GetFunction());
    target->Set(String::NewSymbol("connect_to_fd"),
        FunctionTemplate::New(ConnectToFd)->GetFunction());
    target->Set(String::NewSymbol("get_interface_by_name"),
        FunctionTemplate::New(Interface::GetInterfaceByName)->GetFunction());
    target->Set(String::NewSymbol("display_close"),
        FunctionTemplate::New(DisplayClose)->GetFunction());
    target->Set(String::NewSymbol("display_fileno"),
        FunctionTemplate::New(DisplayFileno)->GetFunction());
    target->Set(String::NewSymbol("display_flush"),
        FunctionTemplate::New(DisplayFlush)->GetFunction());
    target->Set(String::NewSymbol("display_roundtrip"),
        FunctionTemplate::New(DisplayRoundtrip)->GetFunction());

//    target->Set(String::NewSymbol("wl_display_interface"), External::Wrap(NULL));//const_cast<wl_interface*>(&wl_display_interface)));
//
//    target->Set(String::NewSymbol("wl_registry_interface"), External::Wrap((void*)&wl_registry_interface));
//    target->Set(String::NewSymbol("wl_callback_interface"), External::Wrap((void*)&wl_callback_interface));
//    target->Set(String::NewSymbol("wl_compositor_interface"), External::Wrap((void*)&wl_compositor_interface));
//    target->Set(String::NewSymbol("wl_shm_pool_interface"), External::Wrap((void*)&wl_shm_pool_interface));
//    target->Set(String::NewSymbol("wl_shm_interface"), External::Wrap((void*)&wl_shm_interface));
//    target->Set(String::NewSymbol("wl_buffer_interface"), External::Wrap((void*)&wl_buffer_interface));
//    target->Set(String::NewSymbol("wl_data_offer_interface"), External::Wrap((void*)&wl_data_offer_interface));
//    target->Set(String::NewSymbol("wl_data_source_interface"), External::Wrap((void*)&wl_data_source_interface));
//    target->Set(String::NewSymbol("wl_data_device_interface"), External::Wrap((void*)&wl_data_device_interface));
//    target->Set(String::NewSymbol("wl_data_device_manager_interface"), External::Wrap((void*)&wl_data_device_manager_interface));
//    target->Set(String::NewSymbol("wl_shell_interface"), External::Wrap((void*)&wl_shell_interface));
//    target->Set(String::NewSymbol("wl_shell_surface_interface"), External::Wrap((void*)&wl_shell_surface_interface));
//    target->Set(String::NewSymbol("wl_surface_interface"), External::Wrap((void*)&wl_surface_interface));
//    target->Set(String::NewSymbol("wl_seat_interface"), External::Wrap((void*)&wl_seat_interface));
//    target->Set(String::NewSymbol("wl_pointer_interface"), External::Wrap((void*)&wl_pointer_interface));
//    target->Set(String::NewSymbol("wl_keyboard_interface"), External::Wrap((void*)&wl_keyboard_interface));
//    target->Set(String::NewSymbol("wl_touch_interface"), External::Wrap((void*)&wl_touch_interface));
//    target->Set(String::NewSymbol("wl_output_interface"), External::Wrap((void*)&wl_output_interface));
//    target->Set(String::NewSymbol("wl_region_interface"), External::Wrap((void*)&wl_region_interface));
}
NODE_MODULE(wayland_client, Init);
