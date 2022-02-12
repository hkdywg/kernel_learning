
OUT_PATH := $(shell pwd)/out

all: bubblesort selectsort insertsort
	test -d ${OUT_PATH} || mkdir -p ${OUT_PATH}
	mv $^ ${OUT_PATH}

clean:
	rm -f ${OUT_PATH}/*


bubblesort:
	@gcc -g -o bubblesort bubblesort.c

selectsort:
	@gcc -g -o selectsort selectsort.c

insertsort:
	@gcc -g -o insertsort insertsort.c
