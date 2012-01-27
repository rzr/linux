cmd_fs/binfmt_aout.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o fs/binfmt_aout.ko fs/binfmt_aout.o fs/binfmt_aout.mod.o
