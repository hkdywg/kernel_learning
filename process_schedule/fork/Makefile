
.PHONY: all cow thread life_period globalfifo
.PHONY: clean

OUTDIR := out
TOOLCHAIN_BASE := /home/ywg/ywg_workspace/BiscuitOS_arm64/output/linux-5.0-aarch/aarch64-linux-gnu/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
CC := ${TOOLCHAIN_BASE}gcc
AR := ${TOOLCHAIN_BASE}ar

ARCH := arm64

WORK_PATH := /home/ywg/ywg_workspace/BiscuitOS_arm64/output/linux-5.0-aarch
ROOT_FS_USERSPACE := /home/ywg/ywg_workspace/BiscuitOS_arm64/output/linux-5.0-aarch/rootfs/rootfs/user_space
RUN_OS := /home/ywg/ywg_workspace/BiscuitOS_arm64/output/linux-5.0-aarch/RunBiscuitOS.sh
KERNEL_PATH := /home/ywg/ywg_workspace/BiscuitOS_arm64/output/linux-5.0-aarch/linux/linux
MODULE_PATH := $(shell pwd)
GENERATE_FILE := *.o *.ko *.order *.symvers *.mod.c *.mod

define check_outdir
	if [ ! -d ${OUTDIR} ];then \
		mkdir -p ${OUTDIR}; \
	fi
endef


define mv_outfile
	@[ -e $1 ] && [ -d $ ] && mv $1 $2
endef

#$(eval $(call check_outdir))
all: cow thread

cow:cow.c
	$(check_outdir)
	@gcc cow.c -o cow
	$(call mv_outfile,$@,${OUTDIR})

thread:thread.c
	$(check_outdir)
	@gcc thread.c -o thread -pthread
	$(call mv_outfile,$@,${OUTDIR})

life_period:life_period.c
	$(check_outdir)
	@gcc $^ -o $@
	$(call mv_outfile,$@,${OUTDIR})

globalfifo:
	$(check_outdir)
	make -C ${KERNEL_PATH} M=${MODULE_PATH} ARCH=${ARCH} CROSS_COMPILE=${TOOLCHAIN_BASE}  modules
	$(call mv_outfile,${GENERATE_FILE},${OUTDIR})
	
install:
	cp ${OUTDIR}/*.ko ${ROOT_FS_USERSPACE}
	cd ${WORK_PATH}
	${RUN_OS} pack

clean:
	@rm ${OUTDIR}/* -rf
