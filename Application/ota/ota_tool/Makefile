CC=
#CC=${CROSS_COMPILE}

ifeq (${BUILD_OUTPUT_PATH},)
build_output_path = $(shell pwd)
endif

ifneq (${CROSS_COMPILE},)
	CC=${CROSS_COMPILE}
endif

CFLAGS += ${CFLAG} -Iinclude/

srcs_c = $(wildcard *.c)
target = $(patsubst %.c,${build_output_path}/out/%,${srcs_c})

$(warning $(target))

$(target):${srcs_c}
	@echo ${CC}gcc $@
	@mkdir -p $(abspath $(dir $@))
	@$(CC)gcc $(CFLAGS)  -o $@ -lpthread $< sub_src/veeprom.c

all:$(target)


