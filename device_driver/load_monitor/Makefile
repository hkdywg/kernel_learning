obj-m += load_monitor.o

TOOLCHAIN_BASE := /home/ywg/ywg_workspace/BiscuitOS_arm64/output/linux-5.0-aarch/aarch64-linux-gnu/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
#TOOLCHAIN_BASE := /opt/toolchain/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-

CC := ${TOOLCHAIN_BASE}gcc
AR := ${TOOLCHAIN_BASE}ar

ARCH := arm64

WORK_PATH := /home/ywg/ywg_workspace/BiscuitOS_arm64/output/linux-5.0-aarch
ROOT_FS_USERSPACE := /home/ywg/ywg_workspace/BiscuitOS_arm64/output/linux-5.0-aarch/rootfs/rootfs/user_space
RUN_OS := /home/ywg/ywg_workspace/BiscuitOS_arm64/output/linux-5.0-aarch/RunBiscuitOS.sh

KERNEL_PATH := /home/ywg/ywg_workspace/BiscuitOS_arm64/output/linux-5.0-aarch/linux/linux
#KERNEL_PATH := /home/yinwg/ywg_workspace/prj/linux_kernel/build_ark_v4/lib/modules/5.4.40-g66cf445b76/build
MODULE_PATH := $(shell pwd)
OUT_PATH := ${MODULE_PATH}/out
GENERATE_FILE := *.o *.ko *.order *.symvers *.mod.c 

all:
	test -d ${OUT_PATH} || mkdir -p ${OUT_PATH}
	make -C ${KERNEL_PATH} M=${MODULE_PATH} ARCH=${ARCH} CROSS_COMPILE=${TOOLCHAIN_BASE}  modules 
	mv ${GENERATE_FILE} ${OUT_PATH}

install:
	cp ${OUT_PATH}/*.ko ${ROOT_FS_USERSPACE}
	cd ${WORK_PATH}
	${RUN_OS} pack

clean:
	rm -f ${OUT_PATH}/*
