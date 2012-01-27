cmd_crypto/seed.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o crypto/seed.ko crypto/seed.o crypto/seed.mod.o
