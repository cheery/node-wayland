{
    "targets": [
        {
            "target_name": "wayland_client",
            "sources": [
                "src/interface.cc",
                "src/proxy.cc",
                "src/client.cc",
            ],
            "libraries": [
                "-lwayland-client",
            ],
        }
    ]
}
