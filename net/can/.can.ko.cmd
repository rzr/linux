cmd_net/can/can.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o net/can/can.ko net/can/can.o net/can/can.mod.o
