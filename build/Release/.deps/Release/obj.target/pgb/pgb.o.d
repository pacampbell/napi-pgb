cmd_Release/obj.target/pgb/pgb.o := cc '-DNODE_GYP_MODULE_NAME=pgb' '-DUSING_UV_SHARED=1' '-DUSING_V8_SHARED=1' '-DV8_DEPRECATION_WARNINGS=1' '-D_LARGEFILE_SOURCE' '-D_FILE_OFFSET_BITS=64' '-DBUILDING_NODE_EXTENSION' -I/home/paul/.node-gyp/8.11.2/include/node -I/home/paul/.node-gyp/8.11.2/src -I/home/paul/.node-gyp/8.11.2/deps/openssl/config -I/home/paul/.node-gyp/8.11.2/deps/openssl/openssl/include -I/home/paul/.node-gyp/8.11.2/deps/uv/include -I/home/paul/.node-gyp/8.11.2/deps/zlib -I/home/paul/.node-gyp/8.11.2/deps/v8/include -I../`pkg-config --cflags pgb`  -fPIC -pthread -Wall -Wextra -Wno-unused-parameter -m64 -Wall -std=c11 -O3 -fno-omit-frame-pointer  -MMD -MF ./Release/.deps/Release/obj.target/pgb/pgb.o.d.raw   -c -o Release/obj.target/pgb/pgb.o ../pgb.c
Release/obj.target/pgb/pgb.o: ../pgb.c \
 /home/paul/.node-gyp/8.11.2/include/node/node_api.h \
 /home/paul/.node-gyp/8.11.2/include/node/node_api_types.h \
 ../napi_helper.h
../pgb.c:
/home/paul/.node-gyp/8.11.2/include/node/node_api.h:
/home/paul/.node-gyp/8.11.2/include/node/node_api_types.h:
../napi_helper.h:
