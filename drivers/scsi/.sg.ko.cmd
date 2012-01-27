cmd_drivers/scsi/sg.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o drivers/scsi/sg.ko drivers/scsi/sg.o drivers/scsi/sg.mod.o
