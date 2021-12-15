/*
 *    COPYRIGHT NOTICE
 *    Copyright 2020 Horizon Robotics, Inc.
 *    All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <mtd/ubi-user.h>
#include <mtd/mtd-user.h>
#include <errno.h>

#include "veeprom.h"

#define DEBUG
#define WARN
#define ERROR

#define _STRINGIFY(s) #s

#define DEFAULT_UBI_CTRL "/dev/ubi_ctrl"
#define UBI_NUM 10
#define VEEPROM_UBI(x)	"/dev/ubi"_STRINGIFY(x)"_0"
#define SECTOR_SIZE (512)

#define BUFFER_SIZE 2048

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) sizeof(x) / sizeof(x[0])
#endif /* ARRAY_SIZE */

#ifndef GPT_ENTRY_MAX
#define GPT_ENTRY_MAX 128
#define GPT_NAME_LEN_BYTES 72
/*
* Struct created to handle gpt_entry
*/
struct gpt_entry {
    uint64_t part_type_guid_low;
    uint64_t part_type_guid_high;
    uint64_t uuid_low;
    uint64_t uuid_high;
    uint64_t start_lba;
    uint64_t end_lba;
    uint64_t attrs;
    uint16_t part_name[GPT_NAME_LEN_BYTES * sizeof(char) / sizeof(uint16_t)];
};
#endif /* GPT_ENTRY_MAX */

char *device_path = "";
unsigned int start_sector;
unsigned int end_sector;
int fd = -1;
char buffer[BUFFER_SIZE];
char bootmode;

static int sync_flag = SYNC_TO_VEEPROM;

int get_boot_mode(void)
{
	int ret = 0;
	char cmd_buf[64] = { 0 };
	char mode = 0;

	/*
	 * some exception list
	 * which using nor+emmc booting, e.g. mono&quad
	 */
	FILE* fd = fopen("/sys/class/socinfo/board_id", "r");
	if(fd) {
		fread(cmd_buf, 1, 4, fd);
		fclose(fd);
	} else {
		WARN("warning: open socinfo/board_id failed, using emmc mode! \n");
		mode = PIN_2ND_EMMC;
		return mode;
	}

	if(cmd_buf[0] == '3' && cmd_buf[1] == '0' && cmd_buf[2] == '0') {
		mode = PIN_2ND_EMMC;
		return mode;
	}
	/* some exception list end */

	memset(cmd_buf, 0, sizeof(cmd_buf));

	fd = fopen("/sys/class/socinfo/boot_mode", "r");
	if(fd) {
		fread(cmd_buf, 1, 1, fd);
		fclose(fd);
	} else {
		WARN("warning: open socinfo/boot_mode failed, using emmc mode! \n");
		mode = PIN_2ND_EMMC;
	}

	if (cmd_buf[0] == '1')
		mode = PIN_2ND_SPINAND;
	else if (cmd_buf[0] == '5')
		mode = PIN_2ND_SPINOR;
	else
		mode = PIN_2ND_EMMC;

	return mode;
}

int parameter_check(char *c, int len)
{
	int i = 0;

	for (i = 0; i < len; i++) {
		if(c[i] < '0' || c[i] > 'f')
			return 1;
	}

	return 0;
}

static int is_parameter_valid(int offset, int size)
{
	int offset_left = 0;
	int offset_right = (end_sector - start_sector + 1) * SECTOR_SIZE;
	int max_size = offset_right - offset_left;

	if (offset + size > max_size)
		return 0;

	return 1;
}

int get_mtdnum_partname(char* target_nm) {
	int target_mtd_nm = -1;
	char tmp_path[128], mtd_name_buf[128], mtd_num_tmp[16];
	char *name_ptr;
	FILE* mtd_fd;
	struct dirent *mtdx;
	/* use mtd sysfs node for finding the correct partition */
	DIR *mtd_sys = opendir("/sys/class/mtd/");
	if (mtd_sys == NULL) {
		ERROR("MTD system does not exist or is too old!\n");
		return -1;
	}
	while ((mtdx = readdir(mtd_sys)) != NULL) {
		memset(tmp_path, 0, sizeof(tmp_path));
		memset(mtd_name_buf, 0, sizeof(mtd_name_buf));
		snprintf(tmp_path, sizeof(tmp_path),
				"/sys/class/mtd/%s/name", mtdx->d_name);
		mtd_fd = fopen(tmp_path, "r");
		if(mtd_fd) {
			fread(mtd_name_buf, 1, 128, mtd_fd);
			fclose(mtd_fd);
			/* Got the partition, extract mtd number */
			if (!strncmp(mtd_name_buf, target_nm,
							strlen(mtd_name_buf) - 1)) {
				memset(mtd_num_tmp, 0, sizeof(mtd_num_tmp));
				name_ptr = mtdx->d_name;
				while (*name_ptr) {
					if (isdigit(*name_ptr)) {
						snprintf(mtd_num_tmp, sizeof(mtd_num_tmp),
								"%ld",
								strtol(name_ptr, &name_ptr, 10));
					} else {
						name_ptr++;
					}
				}
				target_mtd_nm = atoi(mtd_num_tmp);
				break;
			}
		}
	}
	return target_mtd_nm;
}

/*
* Convert str from gpt part_name to normal string
*/
static void convert_partname(uint16_t *str, char *result) {
	unsigned label_max, label_count = 0;
	/* Naively convert UTF16-LE to 7 bits. */
	label_max = fmin(GPT_NAME_LEN_BYTES - 1, sizeof(str));
	while (label_count < label_max) {
		uint8_t c = str[label_count] & 0xff;
		if (c && !isprint(c))
			c = '!';
		result[label_count] = c;
		label_count++;
	}
}

int veeprom_init(void)
{
	fd = open(device_path, O_RDWR | O_SYNC);
	if (fd < 0) {
		bootmode = get_boot_mode();

		if (bootmode == PIN_2ND_SPINAND) {
			if (access(VEEPROM_UBI(UBI_NUM), F_OK) == -1) {
				struct ubi_attach_req req;
				int ubi_ctrl_fd, ret;
				memset(&req, 0, sizeof(struct ubi_attach_req));
				req.mtd_num = -1;
				req.mtd_num = get_mtdnum_partname("boot");

				if (req.mtd_num < 0) {
					ERROR("Boot partition not found!\n");
					return -1;
				}
				req.ubi_num = UBI_NUM;
				ubi_ctrl_fd = open(DEFAULT_UBI_CTRL, O_RDONLY);
				if (ubi_ctrl_fd == -1) {
					ERROR("UBI Control %s not present in system!\n", DEFAULT_UBI_CTRL);
					return -1;
				}
				ret = ioctl(ubi_ctrl_fd, UBI_IOCATT, &req);
				close(ubi_ctrl_fd);
			}
			device_path = VEEPROM_UBI(UBI_NUM);
		} else if (bootmode == PIN_2ND_SPINOR) {
			char nor_veeprom_path[128];
			int nor_veeprom_mtdnm;

			memset(&nor_veeprom_path, 0, sizeof(nor_veeprom_path));

			nor_veeprom_mtdnm = get_mtdnum_partname("veeprom");

			snprintf(nor_veeprom_path, sizeof(nor_veeprom_path),
					 "/dev/mtd%d", nor_veeprom_mtdnm);
			device_path = nor_veeprom_path;
		} else {
			/* Assume using emmc veeprom */
			char *gpt_dev = "/dev/mmcblk0";
			char mmc_veeprom_path[128];
			int ret = 0;
			char name_translated[GPT_ENTRY_MAX][GPT_NAME_LEN_BYTES] = { 0 };
    		struct gpt_entry gpt_entries[GPT_ENTRY_MAX] = { 0 };
			FILE *gpt_on_disk = fopen(gpt_dev, "r");
			/* Read gpt from mmc */
			/* first lba is mbr, second is gpt header, total 1024 bytes */
			if (!gpt_on_disk) {
				printf("mmc:%s device not found!\n", gpt_dev);
				return -1;
			}
			fseek(gpt_on_disk, 1024, SEEK_SET);
			ret = fread(&gpt_entries, sizeof(struct gpt_entry),
						GPT_ENTRY_MAX, gpt_on_disk);
			if (ret != GPT_ENTRY_MAX) {
				printf("read short: %u\n", ret);
				return -1;
			}
			for (int i = 0; i < ARRAY_SIZE(gpt_entries); i++) {
				convert_partname(gpt_entries[i].part_name, name_translated[i]);
				if (!strcmp(name_translated[i], "veeprom")) {
					snprintf(mmc_veeprom_path, sizeof(mmc_veeprom_path),
							"/dev/mmcblk0p%d", i + 1);
					device_path = mmc_veeprom_path;
				}
			}
			fclose(gpt_on_disk);
		}
		start_sector = VEEPROM_START_SECTOR;
		end_sector = VEEPROM_END_SECTOR;
		DEBUG("dev_path:%s, start_sector=%d, end_sector=%d\n",
			  device_path, start_sector, end_sector);

		fd = open(device_path, O_RDWR | O_SYNC);

		if (fd < 0) {
			ERROR("Error: open %s fail\n", device_path);
			return -1;
		}
	}

	return 0;
}

void veeprom_exit(void)
{
	sync();
	close(fd);
	fd = -1;
}

void veeprom_setsync(int flag)
{
	sync_flag = flag;
}

int veeprom_format(void)
{
	return veeprom_clear(0, 256);
}

int veeprom_read(int offset, char *buf, int size)
{
	unsigned int sector_left = 0;
	unsigned int sector_right = 0;
	unsigned int sector_count = 0;
	unsigned int offset_inner = 0;
	unsigned int remain_inner = 0;
	unsigned int i = 0;
	char vbuf[256] = {0};
	char *pbuf = buf;
	unsigned int origin_size = size;
	unsigned int origin_offset = offset;

	if (get_boot_mode() == PIN_2ND_SPINAND) {
		int ret = 0;
		memset(buffer, 0, sizeof(buffer));
		if (fd == -1)
			veeprom_init();
		ret = read(fd, buffer, sizeof(buffer));
		memcpy(buf, buffer + offset, size);
		veeprom_exit();
		return 0;
	}

	if (!is_parameter_valid(offset, size)) {
		ERROR("Error: parameters invalid\n");
		return -1;
	}

	/* compute sector count */
	sector_left = start_sector + (offset / SECTOR_SIZE);
	sector_right = start_sector + ((offset + size - 1) / SECTOR_SIZE);
	sector_count = sector_right - sector_left + 1;
	DEBUG("sector_left = %d\n", sector_left);
	DEBUG("sector_right = %d\n", sector_right);

	for (i = 0; i < sector_count; ++i) {
		int operate_count = 0;
		memset(buffer, 0, sizeof(buffer));

		if (lseek(fd, (sector_left + i) * SECTOR_SIZE, SEEK_SET) < 0) {
			ERROR("Error: lseek sector %d fail\n", sector_left + i);
			return -1;
		}

		if (read(fd, buffer, sizeof(buffer)) < 0) {
			ERROR("Error: read sector %d fail\n", sector_left + i);
			return -1;
		}

		/* sector number: sector_left + i - start_sector */
		offset_inner = offset - (sector_left + i - start_sector) * SECTOR_SIZE;
		remain_inner = SECTOR_SIZE - offset_inner;
		operate_count = (remain_inner >= size ? size : remain_inner);
		size -= operate_count;
		offset += operate_count;
		DEBUG("%s:offset_inner = %d\n", __FUNCTION__, offset_inner);
		DEBUG("%s:operate_count = %d\n", __FUNCTION__, operate_count);
		memcpy(buf, buffer + offset_inner, operate_count);
		buf += operate_count;
	}

	return 0;
}

int veeprom_write(int offset, const char *buf, int size)
{
	if (get_boot_mode() == PIN_2ND_SPINAND) {
		int ret;
		unsigned int bytes = sizeof(buffer);
		memset(buffer, 0, sizeof(buffer));
		if (fd == -1)
			veeprom_init();
		read(fd, buffer, sizeof(buffer));
		memcpy(buffer + offset, buf, size);
		if (ioctl(fd, UBI_IOCVOLUP, &bytes)) {
			ERROR("Start update volume %s failed, %s!\n",
				   VEEPROM_UBI(UBI_NUM), strerror(errno));
			return -1;
		}
		ret = write(fd, buffer, bytes);
		veeprom_exit();
		return 0;
	}

	unsigned int sector_left = 0;
	unsigned int sector_right = 0;
	unsigned int sector_count = 0;
	unsigned int offset_inner = 0;
	unsigned int remain_inner = 0;
	unsigned int i = 0;
	unsigned int origin_size = size;
	unsigned int origin_offset = offset;
	const char *pbuf = buf;

	if (!is_parameter_valid(offset, size)) {
		ERROR("Error: parameters invalid\n");
		return -1;
	}

	sector_left = start_sector + (offset / SECTOR_SIZE);
	sector_right = start_sector + ((offset + size - 1) / SECTOR_SIZE);
	sector_count = sector_right - sector_left + 1;

	for (i = 0; i < sector_count; ++i) {
		int operate_count = 0;
		memset(buffer, 0, sizeof(buffer));

		if (lseek(fd, (sector_left + i) * SECTOR_SIZE, SEEK_SET) < 0) {
			DEBUG("Error: lseek sector %d fail\n", sector_left + i);
			return -1;
		}

		if (read(fd, buffer, sizeof(buffer)) < 0) {
			DEBUG("Error: read sector %d fail\n", sector_left + i);
			return -1;
		}

		if (bootmode == PIN_2ND_SPINOR) {
			struct erase_info_user argp;
			argp.start = 0;
			argp.length = 1024*64;
			ioctl(fd, MEMERASE, &argp);
		}

		offset_inner = offset - (sector_left + i - start_sector) * SECTOR_SIZE;
		remain_inner = SECTOR_SIZE - offset_inner;
		operate_count = (remain_inner >= size ? size : remain_inner);
		size -= operate_count;
		offset += operate_count;
		DEBUG("%s:offset_inner = %d\n", __FUNCTION__, offset_inner);
		DEBUG("%s:operate_count = %d\n", __FUNCTION__, operate_count);
		memcpy(buffer + offset_inner, buf, operate_count);
		buf += operate_count;

		if (lseek(fd, (sector_left + i) * SECTOR_SIZE, SEEK_SET) < 0) {
			DEBUG("Error: lseek sector %d fail\n", sector_left + i);
			return -1;
		}

		if (write(fd, buffer, sizeof(buffer)) < 0) {
			DEBUG("Error: write sector %d fail\n", sector_left + i);
			return -1;
		}
	}
	return 0;
}

int veeprom_clear(int offset, int size)
{
	if (get_boot_mode() == PIN_2ND_SPINAND) {
		int ret = 0;
		char *buf;
		buf = (char*) malloc(size * sizeof(char));
		memset(buf, 0, sizeof(buf));
		ret = veeprom_write(offset, buf, size);
		free(buf);
		return ret;
	}
	unsigned int sector_left = 0;
	unsigned int sector_right = 0;
	unsigned int sector_count = 0;
	unsigned int offset_inner = 0;
	unsigned int remain_inner = 0;
	unsigned int i = 0;
	unsigned int origin_size = size;
	unsigned int origin_offset = offset;

	if (!is_parameter_valid(offset, size)) {
		ERROR("Error: parameters invalid\n");
		return -1;
	}

	sector_left = start_sector + (offset / SECTOR_SIZE);
	sector_right = start_sector + ((offset + size - 1) / SECTOR_SIZE);
	sector_count = sector_right - sector_left + 1;

	for (i = 0; i < sector_count; ++i) {
		int operate_count = 0;
		memset(buffer, 0, sizeof(buffer));

		if (lseek(fd, (sector_left + i) * SECTOR_SIZE, SEEK_SET) < 0) {
			DEBUG("Error: lseek sector %d fail\n", sector_left + i);
			return -1;
		}

		if (read(fd, buffer, sizeof(buffer)) < 0) {
			DEBUG("Error: read sector %d fail\n", sector_left + i);
			return -1;
		}

		offset_inner = offset - (sector_left + i - start_sector) * SECTOR_SIZE;
		remain_inner = SECTOR_SIZE - offset_inner;
		operate_count = (remain_inner >= size ? size : remain_inner);
		size -= operate_count;
		offset += operate_count;
		DEBUG("%s:offset_inner = %d\n", __FUNCTION__, offset_inner);
		DEBUG("%s:operate_count = %d\n", __FUNCTION__, operate_count);
		memset(buffer + offset_inner, 0, operate_count);

		if (lseek(fd, (sector_left + i) * SECTOR_SIZE, SEEK_SET) < 0) {
			DEBUG("Error: lseek sector %d fail\n", sector_left + i);
			return -1;
		}

		if (write(fd, buffer, sizeof(buffer)) < 0) {
			DEBUG("Error: write sector %d fail\n", sector_left + i);
			return -1;
		}
	}

	return 0;
}

int veeprom_dump(void)
{
	unsigned int j = 0;

	if (lseek(fd, start_sector * SECTOR_SIZE, SEEK_SET) < 0) {
		ERROR("Error: lseek sector %d fail\n", 0);
		return -1;
	}

	printf("veeprom:\n");

	memset(buffer, 0, sizeof(buffer));
	if (read(fd, buffer, VEEPROM_MAX_SIZE) < 0) {
		ERROR("Error: read fail\n");
		return -1;
	} else {
		for (j = 0; j < VEEPROM_MAX_SIZE; ++j) {
			printf("%02x ", buffer[j]);

			if (!((j + 1) % 16))
				printf("\n");
		}
	}

	return 0;
}
