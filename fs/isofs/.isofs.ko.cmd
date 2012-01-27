cmd_fs/isofs/isofs.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o fs/isofs/isofs.ko fs/isofs/isofs.o fs/isofs/isofs.mod.o
