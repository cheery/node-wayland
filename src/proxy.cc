#include <v8.h>
#include <node.h>
#include <node_buffer.h>

using namespace v8;

extern "C" {
    #include <wayland-client.h>
    #include <stdio.h>
    #include <stdlib.h>
}

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
    NODE_SET_PROTOTYPE_METHOD(tpl, "marshal", Marshal);
    constructor = Persistent<Function>::New(tpl->GetFunction());
    target->Set(String::NewSymbol("Proxy"), constructor);
}

Handle<Value> Proxy::New(const Arguments& args) {
    HandleScope scope;
    Proxy* proxy = new Proxy();
    proxy->proxy = (wl_proxy*)External::Unwrap(args[0]);
    proxy->interface = (const struct wl_interface*)External::Unwrap(args[1]);
    proxy->object.interface = proxy->interface;
    proxy->object.implementation = proxy->proxy;
    proxy->object.id = wl_proxy_get_id(proxy->proxy);
    proxy->Wrap(args.This());
    return args.This();
};

void Proxy::Free() {
    //listener.object.Dispose();
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
    wl_proxy_add_dispatched_listener(proxy->proxy, wl_nodejs_proxy_dispatcher,
        proxy->interface->events,
        &proxy->self_ref);
    return scope.Close(Undefined());
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
    union wl_argument *c_args = (union wl_argument*)calloc(nargs, sizeof *c_args);
    int j = 0;
    for (int i = 0; i < nargs; i++) {
//        int nullable = 0;
        if (signature[j] == '?') {
//            nullable = 1;
            j++;
        }
        char which = signature[j++];
        switch (which) {
            case 'i':
                c_args[i].i = args[i+1]->Int32Value();
                break;
            case 'u':
                c_args[i].u = args[i+1]->Uint32Value();
                break;
            case 'f':
                c_args[i].f = args[i+1]->NumberValue();
                break;
            case 's': {
                v8::String::Utf8Value string(args[i+1]);
                c_args[i].s = *string;
                } break;
            case 'o': case 'n': {
                Proxy* object = AsProxy(args[i+1]->ToObject());
                c_args[i].o = &(object->object);
                } break;
            case 'a':
                return ThrowException(String::New("support for byte arrays unimplemented"));
                break;
            case 'h':
                c_args[i].h = args[i+1]->IntegerValue();
                break;
        }
    }
    wl_proxy_marshal_a(proxy->proxy, opcode, c_args);
    free(c_args);
    return scope.Close(Undefined());
}

//    for (i = 0; i < count; ++i) {
//        sig_tmp = get_next_argument(sig_tmp, &arg);
//
//        jobj = (*env)->GetObjectArrayElement(env, jargs, i);
//        if ((*env)->ExceptionCheck(env))
//            goto free_args;
//
//        switch(arg.type) {
//        case 'o':
//            args[i].o = (*object_conversion)(env, jobj);
//            break;
//        case 'n':
//            /* new_id types are actually expected to be passed in as objects */
//            args[i].o = (*object_conversion)(env, jobj);
//            break;
//        case 'a':
//            args[i].a = malloc(sizeof(struct wl_array));
//            if (args[i].a == NULL) {
//                wl_jni_throw_OutOfMemoryError(env, NULL);
//                goto free_args;
//            }
//
//            args[i].a->alloc = 0;
//            args[i].a->data = (*env)->GetDirectBufferAddress(env, jobj);
//            args[i].a->size = (*env)->GetDirectBufferCapacity(env, jobj);
//            break;
//        case 'h':
//            args[i].h = wl_jni_unbox_integer(env, jobj);
//            break;
//        }
//
//
//
int
Proxy::wl_nodejs_proxy_dispatcher(const void *data, void *target, uint32_t opcode,
        const struct wl_message *message, union wl_argument *args)
{
//    SelfRef* ref = (SelfRef*)target;
//    Proxy* proxy = AsProxy(ref->object);

    printf("proxy has responded with %s(%s)\n", message->name, message->signature);


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
