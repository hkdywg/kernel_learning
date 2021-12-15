#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "veeprom.h"

static struct option longopts[] = {
    {"updateflag", no_argument, NULL, 'u'},
    {"resetreason", no_argument, NULL, 'r'},
    {0, 0, 0, 0},
};

static char *shortops = "hgs:";

void help(void)
{
    printf("Usage: ota_tool [Options] [sg] <values>\n");
    printf("Example: \n");
    printf("        ota_tool --resetreason -s normal\n");
    printf("Options: \n");
    printf("         --resetreason  system reset reason\n");
    printf("         --updateflag   system upgrade flag\n");
    printf("sg: \n");
    printf("         -s set system flag\n");
    printf("         -g get system flag\n");
}

static int updateflag_get()
{
    int ret;
    char flag;

    ret = veeprom_read(VEEPROM_UPDATE_FLAG_OFFSET, &flag, VEEPROM_UPDATE_FLAG_SIZE);
    if(ret < 0)
    {
        perror("read veeprom failed!\n");
        veeprom_exit();
        return -1;
    }
    printf("%d\n", flag);
    return 0;
}

static int updateflag_set(char *flag)
{
    int ret;
    char set_flag, read_flag;

    if(parameter_check(flag, strlen(flag)))
    {
        perror("the paramter %s is illegal !\n");
        help();
        veeprom_exit();
        return -1;
    }
    set_flag = atoi(flag);
    if((ret = veeprom_write(VEEPROM_UPDATE_FLAG_OFFSET, &set_flag, VEEPROM_UPDATE_FLAG_SIZE)) < 0)
    {
        perror("write eeprom failed!\n");
        veeprom_exit();
        return -1;
    }
    if((ret = veeprom_read(VEEPROM_UPDATE_FLAG_OFFSET, &read_flag, VEEPROM_UPDATE_FLAG_SIZE)) < 0)
    {
        perror("read eeprom failed!\n");
        veeprom_exit();
        return -1;
    }
    if(set_flag != read_flag)
    {
        perror("verify failed!\n");
        veeprom_exit();
        return -1;
    }
    return 0;
}

static int resetreason_para_check(char *para)
{
    char buffer[64];

    strcpy(buffer, para);

    if ((strcmp(buffer, "normal") !=0) && ((strcmp(buffer, "recovery") !=0))
        && (strcmp(buffer, "uboot") !=0) && (strcmp(buffer, "boot") !=0)
        && (strcmp(buffer, "system") !=0) && (strcmp(buffer, "all") !=0)) 
     {
        perror("parameter  is illegal!\n");
        return -1;
     }
    else
        return 0;
}

static int resetreason_read()
{
    int ret;
    char buffer[32];

    if((ret = veeprom_read(VEEPROM_RESET_REASON_OFFSET, buffer, VEEPROM_RESET_REASON_SIZE)) < 0)
    {
        perror("read veerom failed!\n");
        return -1;
    }
    printf("%s\n", buffer);
    return 0;
}


static char resetreason_write(char *para)
{
    int ret;
    char buffer[32];

    if((ret = resetreason_para_check(para)) < 0)
        return -1;

    ret = veeprom_write(VEEPROM_RESET_REASON_OFFSET, para, VEEPROM_RESET_REASON_SIZE);
    if(ret < 0)
    {
        perror("write veeprom failed!\n");
        return -1;
    }
    ret = veeprom_read(VEEPROM_RESET_REASON_OFFSET, buffer, VEEPROM_RESET_REASON_SIZE);
    if(ret < 0)
    {
        perror("read veeprom failed!\n");
        return -1;
    }
    if(strcmp(buffer, para) != 0)
    {
        perror("verify failed!\n");
        return -1;
    }
    return 0;
}


int main(int argc, char **argv[])
{
    int opt;
    int ret;
    char para[32];
    int function_index = 0x00;

    if(veeprom_init() < 0)
    {
        printf("veeprom_init error\n");
        return -1;
    }

    if(argc < 2)
    {
        help();
        return -1;
    }
    
    while((opt=getopt_long(argc, argv, shortops, longopts, NULL)) != -1)
    {
        switch(opt)
        {
            case 'u':
                    function_index = function_index | 0x00;
                break;
            case 'r':
                    function_index = function_index | 0x10;
                break;
            case 's':
                    function_index = function_index | 0x01;
                    strcpy(para, optarg);
                break;
            case 'g':
                    function_index = function_index | 0x00;
                break;
        }
    }

    if(function_index == 0x00)
       updateflag_get();
    else if(function_index == 0x01)
       updateflag_set(para);
    else if(function_index == 0x10)
       resetreason_read();
    else if(function_index == 0x11)
       resetreason_write(para);


    veeprom_exit();
    return  0;
}
