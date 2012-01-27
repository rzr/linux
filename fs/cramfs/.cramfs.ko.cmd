cmd_fs/cramfs/cramfs.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o fs/cramfs/cramfs.ko fs/cramfs/cramfs.o fs/cramfs/cramfs.mod.o
