CC := gcc

SRC := single_list.c
CFLAGS := -I./


all: single_list

single_list: ${SRC}
	${CC} ${SRC} ${CFLAGS} -o $@	
	@mv $@ out

clean:
	rm *.o single_list
