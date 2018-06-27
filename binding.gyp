{
    "targets": [{
        "target_name": "pgb",
        "sources": ["pgb.c"],
        "include_dirs": ["`pkg-config --cflags pgb`"],
        "libraries": ["`pkg-config --libs pgb`"],
        "cflags": [
            "-Wall",
            "-Werror",
            "-std=c11"
        ]
    }]
}
