cmd_fs/fuse/fuse.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o fs/fuse/fuse.ko fs/fuse/fuse.o fs/fuse/fuse.mod.o
