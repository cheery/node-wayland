#include <v8.h>
#include <node.h>
#include <node_buffer.h>

using namespace v8;

extern "C" {
    #include <wayland-client.h>
    #include <wayland-util.h>
    #include <stdio.h>
    #include <stdlib.h>
}

#include "array.h"
#include "interface.h"
#include "proxy.h"

Persistent<Function> Proxy::constructor;

void Proxy::Init(Handle<Object> target) {
    Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    tpl->SetClassName(String::NewSymbol("Proxy"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    NODE_SET_PROTOTYPE_METHOD(tpl, "destroy", Destroy);
    NODE_SET_PROTOTYPE_METHOD(tpl, "get_id", GetId);
    NODE_SET_PROTOTYPE_METHOD(tpl, "get_class", GetClass);
    NODE_SET_PROTOTYPE_METHOD(tpl, "create", Create);
    NODE_SET_PROTOTYPE_METHOD(tpl, "listen", Listen);
    NODE_SET_PROTOTYPE_METHOD(tpl, "spy", Spy);
    NODE_SET_PROTOTYPE_METHOD(tpl, "marshal", Marshal);
    constructor = Persistent<Function>::New(tpl->GetFunction());
    target->Set(String::NewSymbol("Proxy"), constructor);
}

Handle<Value> Proxy::New(const Arguments& args) {
    HandleScope scope;
    Proxy* proxy = new Proxy();
    proxy->proxy = (wl_proxy*)External::Unwrap(args[0]);
    proxy->interface = (const struct wl_interface*)External::Unwrap(args[1]);
    proxy->listener = NULL;
    proxy->paddle = (struct paddle*)malloc(sizeof(struct paddle*));
    proxy->paddle->object = Persistent<Object>::New(args.This());
    proxy->Wrap(args.This());
    wl_proxy_set_user_data(proxy->proxy, proxy->paddle);
    return args.This();
};

void Proxy::Free() {
    if (listener) {
        listener->value.Dispose();
        free(listener);
        listener = NULL;
    }
    if (paddle) {
        paddle->object.Dispose();
        free(paddle);
    }
    proxy = NULL;
}

Handle<Value> Proxy::Destroy(const Arguments& args) {
    HandleScope scope;
    Proxy* proxy = AsProxy(args.This());
    if (proxy == NULL) return ThrowException(String::New("proxy is dead"));
    wl_proxy_destroy(proxy->proxy);
    proxy->Free();
    return scope.Close(Undefined());
};

Handle<Value> Proxy::GetId(const Arguments& args) {
    HandleScope scope;
    Proxy* proxy = AsProxy(args.This());
    if (proxy == NULL) return ThrowException(String::New("proxy is dead"));
    uint32_t id = wl_proxy_get_id(proxy->proxy);
    return scope.Close(Integer::New(id));
};

Handle<Value> Proxy::GetClass(const Arguments& args) {
    HandleScope scope;
    Proxy* proxy = AsProxy(args.This());
    if (proxy == NULL) return ThrowException(String::New("proxy is dead"));
    const char* classname = wl_proxy_get_class(proxy->proxy);
    return scope.Close(String::New(classname));
};

Handle<Value> Proxy::Create(const Arguments& args) {
    HandleScope scope;
    Proxy* proxy = AsProxy(args.This());
    if (proxy == NULL) return ThrowException(String::New("proxy is dead"));
    const struct wl_interface* interface = Interface::Unwrap(args[0]->ToObject());
    wl_proxy* child_proxy = wl_proxy_create(proxy->proxy, interface);
    const unsigned argc = 2;
    Handle<Value> argv[argc] = {
        External::Wrap(child_proxy),
        External::Wrap((void*)interface),
    };
    Local<Object> instance = Proxy::constructor->NewInstance(argc, argv);
    return scope.Close(instance);
}

Handle<Value> Proxy::Listen(const Arguments& args) {
    HandleScope scope;
    Proxy* proxy = AsProxy(args.This());
    if (proxy == NULL) return ThrowException(String::New("proxy is dead"));
    if (proxy->listener != NULL) return ThrowException(String::New("listener added already"));
    proxy->listener = (struct listener*)malloc(sizeof(struct listener));
    proxy->listener->value = Persistent<Value>::New(args[0]);
    wl_proxy_add_dispatched_listener(proxy->proxy, wl_nodejs_proxy_dispatcher,
        proxy->listener,
        NULL);
    return scope.Close(Undefined());
}

Handle<Value> Proxy::Spy(const Arguments& args) {
    HandleScope scope;
    Proxy* proxy = AsProxy(args.This());
    if (proxy == NULL) return ThrowException(String::New("proxy is dead"));
    proxy->paddle->object.Dispose();
    proxy->paddle->object = Persistent<Object>::New(args[0]->ToObject());
    return scope.Close(Undefined());
}

static
int wl_argument_from_value(union wl_argument* arg, Local<Value> value, int which, int nullable) {
    switch (which) {
        case 'i': arg->i = value->Int32Value(); break;
        case 'u': arg->u = value->Uint32Value(); break;
        case 'f': arg->f = wl_fixed_from_double(value->NumberValue()); break;
        case 's':
        {
            // Copy to memory if things break down.
            v8::String::Utf8Value string(value);
            arg->s = *string;
        } break;
        case 'o':
        case 'n':
        {
            if (value->IsUndefined() || value->IsNull()) {
                arg->o = NULL;
                if (!nullable) return 0;
            } else {
                Proxy* proxy = Proxy::AsProxy(value->ToObject());
                struct wl_object* object = (struct wl_object*)malloc(sizeof *object);
                object->interface = proxy->interface;
                object->implementation = proxy->proxy;
                object->id = wl_proxy_get_id(proxy->proxy);
                arg->o = object;
            }
        } break;
        case 'a':
            // Remember to unallocate this.
            arg->a = (struct wl_array*)malloc(sizeof(struct wl_array));
            arg->a->alloc = 0;
            arg->a->data = ArrayBuffer::GetData(value->ToObject(), &arg->a->size);
            break;
        case 'h': arg->h = value->IntegerValue(); break;
        default:
            return 0;
    }
    return 1;
}

static
void wl_cleanup_argument(union wl_argument* arg, int which) {
    switch (which) {
        case 'i': break;
        case 'u': break;
        case 'f': break;
        case 's': break;
        case 'o':
        case 'n': free((void*)arg->o); break;
        case 'a': free((void*)arg->a); break;
        case 'h': break;
    }
}

Local<Value> wl_argument_to_value(union wl_argument* arg, int which) {
    switch (which) {
        case 'i': return Integer::New(arg->i);
        case 'u': return Integer::NewFromUnsigned(arg->u);
        case 'f': return Number::New(wl_fixed_to_double(arg->f));
        case 's': return String::New(arg->s);
        case 'o':
        case 'n': {
            if (arg->o == NULL) {
                return *Null();
            } else if (arg->o->implementation == NULL) { // fix this, there most likely is a proxy that can be located for this entry.
                return Integer::New(arg->o->id);
            } else {
                struct paddle* paddle = (struct paddle*)wl_proxy_get_user_data((wl_proxy*)arg->o->implementation);
                return *paddle->object;
            }
        }
        case 'a': return ArrayBuffer::New(arg->a->size, arg->a->data);
        case 'h': return Integer::New(arg->h);
    }
    return *Undefined();
}

Handle<Value> Proxy::Marshal(const Arguments& args) {
    HandleScope scope;
    Proxy* proxy = AsProxy(args.This());
    if (proxy == NULL) return ThrowException(String::New("proxy is dead"));
    uint32_t opcode = args[0]->Uint32Value();
    const char* signature = proxy->interface->methods[opcode].signature;
    int nargs = 0;
    for (int i = 0; signature[i]; i++) {
        if (signature[i] != '?') nargs++;
    }
    if (nargs != args.Length() - 1) {
        return ThrowException(String::New("argc doesn't match the signature"));
    }
    union wl_argument *argv = (union wl_argument*)calloc(nargs, sizeof *argv);
    int j = 0;
    int arg_ok = 1;
    for (int i = 0; i < nargs; i++) {
        int nullable = 0;
        if (signature[j] == '?') { nullable = 1; j++; }
        arg_ok &= wl_argument_from_value(argv+i, args[i+1], signature[j++], nullable);
    }
    if (arg_ok) wl_proxy_marshal_a(proxy->proxy, opcode, argv);
    j = 0;
    for (int i = 0; i < nargs; i++) {
        /*int nullable = 0;*/
        if (signature[j] == '?') { /*nullable = 1;*/ j++; }
        wl_cleanup_argument(argv+i, signature[j++]);
    }
    free(argv);
    if (!arg_ok) return ThrowException(String::New("bad argument list"));
    return scope.Close(Undefined());
}

int
Proxy::wl_nodejs_proxy_dispatcher(const void *data, void *target, uint32_t opcode,
        const struct wl_message *message, union wl_argument *args)
{
    struct listener* listener = (struct listener*)data;
//    printf("proxy data %p with target %p and opcode %i\n", data, target, opcode);
//    SelfRef* ref = (SelfRef*)target;
//    Proxy* proxy = AsProxy(ref->object);
    Persistent<Function> callback = Persistent<Function>::Cast(listener->value);

    unsigned argc = 1;

    for (int i = 0; message->signature[i]; i++) {
        char which = message->signature[i];
        if (which != '?') argc++;
    }

    Local<Value> argv[argc];
    argv[0] = String::New(message->name);
    int j = 0;
    for (unsigned i = 0; i < argc; i++) {
        /*int nullable = 0;*/
        if (message->signature[j] == '?') { /*nullable = 1;*/ j++; }
        argv[i+1] = wl_argument_to_value(args+i, message->signature[j++]);
    }
    callback->Call(Context::GetCurrent()->Global(), argc, argv);



//    const char *signature;
//    int nargs, nrefs;
//
//    jvalue *jargs;
//    JNIEnv *env;
//    jobject jlistener, jproxy;
//    jmethodID mid;
//
//    proxy = target;
//
//    env = wl_jni_get_env();
//
//    /* Count the number of arguments and references */
//    nargs = 0;
//    nrefs = 0;
//    for (signature = message->signature; *signature != '\0'; ++signature) {
//        switch (*signature) {
//        /* These types will require references */
//        case 'f':
//        case 's':
//        case 'o':
//        case 'a':
//        case 'n':
//            ++nrefs;
//        /* These types don't require references */
//        case 'u':
//        case 'i':
//        case 'h':
//            ++nargs;
//            break;
//        case '?':
//            break;
//        }
//    }
//
//    jargs = malloc((nargs + 1) * sizeof *jargs);
//    if (jargs == NULL) {
//        wl_jni_throw_OutOfMemoryError(env, NULL);
//        goto exception_check;
//    }
//
//    if ((*env)->PushLocalFrame(env, nrefs + 2) < 0)
//        goto exception_check;
//
//    jproxy = wl_jni_proxy_to_java(env, proxy);
//    if ((*env)->ExceptionCheck(env)) {
//        goto pop_local_frame;
//    } else if (jproxy == NULL) {
//        wl_jni_throw_NullPointerException(env, "Proxy should not be null");
//        goto pop_local_frame;
//    }
//
//    jlistener = (*env)->GetObjectField(env, jproxy, Proxy.listener);
//    if ((*env)->ExceptionCheck(env))
//        goto pop_local_frame;
//
//    wl_jni_arguments_to_java(env, args, jargs + 1, message->signature, nargs,
//            JNI_TRUE,
//            (jobject(*)(JNIEnv *, struct wl_object *))&wl_jni_proxy_to_java);
//
//    if ((*env)->ExceptionCheck(env))
//        goto pop_local_frame;
//
//    jargs[0].l = jproxy;
//    mid = ((jmethodID *)data)[opcode];
//    (*env)->CallVoidMethodA(env, jlistener, mid, jargs);
//
//pop_local_frame:
//    (*env)->PopLocalFrame(env, NULL);
//    free(jargs);
//
//exception_check:
//    if ((*env)->ExceptionCheck(env))
//        return -1;
//    else
//        return 0;

    return 0;
}
