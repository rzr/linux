cmd_crypto/vmac.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o crypto/vmac.ko crypto/vmac.o crypto/vmac.mod.o
