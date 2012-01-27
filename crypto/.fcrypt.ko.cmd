cmd_crypto/fcrypt.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o crypto/fcrypt.ko crypto/fcrypt.o crypto/fcrypt.mod.o
