cmd_crypto/zlib.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o crypto/zlib.ko crypto/zlib.o crypto/zlib.mod.o
