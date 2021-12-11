#!/bin/sh

#check ota bin is exit
function check_ota_bin()
{
    if [ ! -f ${ota_tool} ];then
        echo "[OTA_INFO] Error: file ${ota_tool} not found!"
        exit 1
    fi
}

function set_count()
{
    tmp=$(${ota_tool} --count $1)
}

function set_updateflag()
{
    if [ -z $1 ];then
        echo "[OTA_INFO] Error: upflag paramter not set"
        return 1
    fi
    
    tmp=$(${ota_tool} --updateflag s $1)
}

function get_updateflag()
{
    update_flag=$(${ota_tool} --updateflag g )
}

function get_updatemode()
{
    ota_upmode=$(${ota_tool} --updatemode g)
}

function set_resetreason()
{
    if [ -z $1 ];then
        echo "[OTA_INFO] Error: upflag paramter not set"
        return 1
    fi
    tmp=$(${ota_tool} --resetreason s $1)
}

function get_resetreason()
{
    resetreason=$(${ota_tool} -- resetreason g) 
}

function mkdir_log_file()
{
    if [ ! -d ${ota_logpath} ] && [ x"${ota_bootmod}" != x"nor_flash" ];then
        local userdata_free_space tmp_free_space
        local log_need_space="150"
        userdata_free_space=$(df -k /userdata/ | grep "userdata" | awk '{print $4}')
        tmp_free_space=$(df -k /tmp/ | grep "tmp" | awk "{print $4}")

        if [ "$userdata_free_space" -gt "$log_need_space" ];then
            ota_logpath="/userdata/cache"
        elif [ "$userdata_free_space" -gt "$log_need_space" ];then
            rm -rf /userdata/cache
            ota_logpath="/userdata/cache"
        else
            echo "[OTA_INFO] Error: not enough free space"
            exit 1
        fi
    fi

    [ x"${ota_bootmode}" = x"nor_flash" ] && ota_logpath="/tmp/cache"
    
    if [ ! -d ${ota_logpath} ];then
        mkdir -p ${ota_logpath}
    fi

    for i in ${progress} ${ota_info} ${update_result}
    do
        touch $i
    done
}

# main function
function all()
{
    set_count 0

    get_resetreason

    # ota pipe init
    if [ x"${resetreason}" = x"recovery" ];then 
        ota_utility
    else
        rm -rf $command
        ota_utility &
    fi

    # get ota upmode from veeprom
    get_updatemode

    # get system boot reason
    get_resetreason

    # synchronize data to device
    sync

}




ota_logpath="/userdata/cache"
progress="${ota_logpath}/progress"
ota_info="${ota_logpath}/info"
update_result="${ota_logpath}/result"
upgrade_file_path="/userdata/cache"

ota_tool="/usr/bin/ota_tool"

update_flag=0
ota_upmode="AB"
ab_partition_status=0
reboot_flag=0
resetreason="normal"
ota_bootmode="emmc"

#check utility
check_ota_bin

all

sync
