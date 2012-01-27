cmd_crypto/anubis.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o crypto/anubis.ko crypto/anubis.o crypto/anubis.mod.o
