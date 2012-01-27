cmd_fs/udf/udf.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o fs/udf/udf.ko fs/udf/udf.o fs/udf/udf.mod.o
