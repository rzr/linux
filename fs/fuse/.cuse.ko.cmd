cmd_fs/fuse/cuse.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o fs/fuse/cuse.ko fs/fuse/cuse.o fs/fuse/cuse.mod.o
