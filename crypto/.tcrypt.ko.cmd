cmd_crypto/tcrypt.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o crypto/tcrypt.ko crypto/tcrypt.o crypto/tcrypt.mod.o
