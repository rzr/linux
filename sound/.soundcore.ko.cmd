cmd_sound/soundcore.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o sound/soundcore.ko sound/soundcore.o sound/soundcore.mod.o
