class Proxy : public node::ObjectWrap {
    const struct wl_interface* interface;
    
    static int
    wl_nodejs_proxy_dispatcher(const void *data, void *target, uint32_t opcode,
        const struct wl_message *message, union wl_argument *args);
public:
    static inline Proxy* AsProxy(Handle<Object> object){
        Proxy* proxy = ObjectWrap::Unwrap<Proxy>(object);
        if (proxy->proxy == NULL) return NULL;
        return proxy;
    };

    void Free();

    static void Init(Handle<Object> target);
    static Handle<Value> New(const Arguments& args);
    static Handle<Value> Destroy(const Arguments& args);
    static Handle<Value> GetId(const Arguments& args);
    static Handle<Value> GetClass(const Arguments& args);
    static Handle<Value> Create(const Arguments& args);
    static Handle<Value> Listen(const Arguments& args);
    static Handle<Value> Marshal(const Arguments& args);

    static Persistent<Function> constructor;

    struct wl_object object;
    struct wl_proxy* proxy;
};
