cmd_crypto/authenc.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o crypto/authenc.ko crypto/authenc.o crypto/authenc.mod.o
