cmd_crypto/rmd256.ko := arm-linux-gnueabi-ld -EL -r  -T /usr/local/src/project/linux/linux/scripts/module-common.lds --build-id  -o crypto/rmd256.ko crypto/rmd256.o crypto/rmd256.mod.o
