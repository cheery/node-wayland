class Interface : public node::ObjectWrap {
    const struct wl_interface* interface;
public:
    static inline const struct wl_interface* Unwrap(Handle<Object> object) {
        Interface* wli = ObjectWrap::Unwrap<Interface>(object);
        return wli->interface;
    };
    static void Init(Handle<Object> target);
    static Handle<Value> New(const Arguments& args);
    static Handle<Value> GetName(const Arguments& args);
    static Handle<Value> GetInterfaceByName(const Arguments& args);

    static Persistent<Function> constructor;
};
