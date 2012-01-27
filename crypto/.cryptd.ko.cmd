cmd_crypto/cryptd.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o crypto/cryptd.ko crypto/cryptd.o crypto/cryptd.mod.o
