cmd_drivers/uio/uio.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o drivers/uio/uio.ko drivers/uio/uio.o drivers/uio/uio.mod.o
