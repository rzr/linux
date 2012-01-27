cmd_lib/raid6/altivec2.c := awk -f/usr/local/src/project/linux/linux/lib/raid6/unroll.awk -vN=2 < lib/raid6/altivec.uc > lib/raid6/altivec2.c || ( rm -f lib/raid6/altivec2.c && exit 1 )
