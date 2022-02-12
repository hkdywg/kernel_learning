
OUT_PATH := $(shell pwd)/out

all: bubblesort
	test -d ${OUT_PATH} || mkdir -p ${OUT_PATH}
	mv $< ${OUT_PATH}

clean:
	rm -f ${OUT_PATH}/*


bubblesort:
	@gcc -g -o bubblesort bubblesort.c
