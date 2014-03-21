#include <v8.h>
#include <node.h>
#include <node_buffer.h>

using namespace v8;

extern "C" {
    #include <wayland-client.h>
    #include <stdio.h>
    #include <string.h>
}

#include "array.h"

static Persistent<Function> arraybuffer_constructor;

namespace ArrayBuffer {
    void DemandInit() {
        if (arraybuffer_constructor.IsEmpty()) {
            Local<Object> global = Context::GetCurrent()->Global();
            Local<Value> val = global->Get(String::New("ArrayBuffer"));
            assert(!val.IsEmpty() && "type not found: ArrayBuffer");
            assert(val->IsFunction() && "not a constructor: ArrayBuffer");
            arraybuffer_constructor = Persistent<Function>::New(val.As<Function>());
        }
    };
    Local<Object> New(size_t size, void* data) {
        DemandInit();
        Local<Value> arg_size = Integer::NewFromUnsigned(size);
        Local<Object> array = arraybuffer_constructor->NewInstance(1, &arg_size);
        if (data != NULL) SetData(array, size, data);
        return array;
    }
    Local<Object> Wrap(size_t size, void* data) {
        DemandInit();
        Local<Value> arg_size = Integer::NewFromUnsigned(size);
        Local<Object> array = arraybuffer_constructor->NewInstance(1, &arg_size);
        array->SetIndexedPropertiesToExternalArrayData(data, kExternalByteArray, size);
        return array;
    }

    void SetData(Local<Object> array, size_t size, void* data) {
        void* dst = array->GetIndexedPropertiesExternalArrayData();
        memcpy(dst, data, size);
    }

    void* GetData(Local<Object> array, size_t* size) {
        *size = array->Get(String::NewSymbol("byteLength"))->IntegerValue();
        return array->GetIndexedPropertiesExternalArrayData();
    }
}
