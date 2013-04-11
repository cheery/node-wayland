namespace ArrayBuffer {
    Local<Object> New(size_t size, void* data);
    void SetData(Local<Object> array, size_t size, void* data);
    void* GetData(Local<Object> array, size_t* size);
}
