cmd_lib/raid6/altivec8.c := awk -f/usr/local/src/project/linux/linux/lib/raid6/unroll.awk -vN=8 < lib/raid6/altivec.uc > lib/raid6/altivec8.c || ( rm -f lib/raid6/altivec8.c && exit 1 )
