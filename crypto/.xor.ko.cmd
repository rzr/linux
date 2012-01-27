cmd_crypto/xor.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o crypto/xor.ko crypto/xor.o crypto/xor.mod.o
