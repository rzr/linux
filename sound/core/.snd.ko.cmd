cmd_sound/core/snd.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o sound/core/snd.ko sound/core/snd.o sound/core/snd.mod.o
