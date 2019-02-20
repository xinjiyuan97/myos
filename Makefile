boot.bin: ./ipl/boot.s Makefile
	nasm -f bin ./ipl/boot.s -o boot.bin

init.bin: ./ipl/init.s Makefile
	nasm -f bin ./ipl/init.s -o init.bin

boot.img: boot.bin init.bin
	./ExtUtils/createImg.x -B boot.bin init.bin -o boot.img
	# ./ExtUtils/edimg imgin:./ExtUtils/fdimg0at.tek \
	# 	wbinimg src:boot.bin len:512 from:0 to:0 \
	#	copy from:init.bin to:@: \
	#	imgout:boot.img

install:
	make -r boot.img