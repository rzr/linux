cmd_fs/binfmt_misc.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o fs/binfmt_misc.ko fs/binfmt_misc.o fs/binfmt_misc.mod.o
