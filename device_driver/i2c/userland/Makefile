#TOOLCHAIN_BASE := /home/ywg/ywg_workspace/BiscuitOS_arm64/output/linux-5.0-aarch/aarch64-linux-gnu/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
TOOLCHAIN_BASE := /opt/toolchain/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-

CC := ${TOOLCHAIN_BASE}gcc
AR := ${TOOLCHAIN_BASE}ar

SOURCE_FILE := i2c.c
TARGET := i2c_demo.out

all:
	${CC}  ${SOURCE_FILE} -o ${TARGET}

clean:
	@rm ${TARGER}
