/*
 *    COPYRIGHT NOTICE
 *    Copyright 2020 Horizon Robotics, Inc.
 *    All rights reserved.
 */
#ifndef _VEEPROM_H_
#define _VEEPROM_H_


/* define copy from include/veeprom.h in uboot */
#define VEEPROM_START_SECTOR (0)
#define VEEPROM_END_SECTOR (0)

#define VEEPROM_BOARD_ID_OFFSET		0
#define VEEPROM_BOARD_ID_SIZE		2
#define VEEPROM_MACADDR_OFFSET      2
#define VEEPROM_MACADDR_SIZE        6
#define VEEPROM_UPDATE_FLAG_OFFSET  8
#define VEEPROM_UPDATE_FLAG_SIZE    1
#define VEEPROM_RESET_REASON_OFFSET	9
#define VEEPROM_RESET_REASON_SIZE	8

#define VEEPROM_IPADDR_OFFSET       17
#define VEEPROM_IPADDR_SIZE         4
#define VEEPROM_IPMASK_OFFSET       21
#define VEEPROM_IPMASK_SIZE         4
#define VEEPROM_IPGATE_OFFSET       25
#define VEEPROM_IPGATE_SIZE         4
#define VEEPROM_UPDATE_MODE_OFFSET  29
#define VEEPROM_UPDATE_MODE_SIZE    8
#define VEEPROM_ABMODE_STATUS_OFFSET  37
#define VEEPROM_ABMODE_STATUS_SIZE    1
#define VEEPROM_COUNT_OFFSET  		38
#define VEEPROM_COUNT_SIZE    		1
#define VEEPROM_SOMID_OFFSET  		39
#define VEEPROM_SOMID_SIZE    		1
#define VEEPROM_PERI_PLL_OFFSET     40
#define VEEPROM_PERI_PLL_SIZE       16
#define VEEPROM_LOG_MASK_OFFSET     56
#define VEEPROM_LOG_MASK_SIZE       10
#define VEEPROM_SECCHIP_OFFSET	    66
#define VEEPROM_SECCHIP_SIZE        64
#define VEEPROM_CAMTYPE_OFFSET      130
#define VEEPROM_CAMTYPE_SIZE        1
#define VEEPROM_COUNT_FLAG_OFFSET   131
#define VEEPROM_COUNT_FLAG_SIZE     1
#define VEEPROM_X3_BOARD_ID_OFFSET  132
#define VEEPROM_X3_BOARD_ID_SIZE    4

#define VEEPROM_DUID_OFFSET         220
#define VEEPROM_DUID_SIZE           32
/*
 * Reserver 4 Byte in the end (252~255)
 */

#define VEEPROM_MAX_SIZE			256
/* end copy from include/configs/socfpga_arria10.h in uboot */

#define SYNC_TO_VEEPROM  0
#define SYNC_TO_EEPROM   1
#define SYNC_NO          2

/* boot mode */
#define PIN_2NDBOOT_SEL(x,y)		((((x) & 0x1) << 1) | ((y) & 0x1))
#define PIN_2ND_EMMC		0x0
#define PIN_2ND_SPINAND			0x1
#define PIN_2ND_AP			0x2
#define PIN_2ND_UART		0x3
#define PIN_2ND_USB         0x4
#define PIN_2ND_SPINOR      0x5

#define CHECK_SUM_OFFSET        0xc
#define BOARD_ID_OFFSET         0xc0

/* ddr frequency */
#define DDR_FREQC_667   0x1
#define DDR_FREQC_1600  0x2
#define DDR_FREQC_2133  0x3
#define DDR_FREQC_2666  0x4
#define DDR_FREQC_3200  0x5
#define DDR_FREQC_3733  0x6
#define DDR_FREQC_4266  0x7
#define DDR_FREQC_1866  0x8
#define DDR_FREQC_2400  0x9
#define DDR_FREQC_100   0xa

/* ddr ecc */
#define MMC_DDR_PARTITION_OFFSET    0x426
#define MMC_DDR_ECC_OFFSET          8

int parameter_check(char *c, int len);

int veeprom_init(void);

void veeprom_exit(void);

int veeprom_format(void);

void veeprom_setsync(int flag);

int veeprom_read(int offset, char *buf, int size);

int veeprom_write(int offset, const char *buf, int size);

int veeprom_clear(int offset, int size);

int veeprom_dump(void);

#endif
