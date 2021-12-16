#!/bin/sh

function usage()
{
    echo "Using:"
    echo "      ./up_partition.sh partition_name file_name"
    echo "      partition_name: all | uboot | boot | system"
    echo "      file_name: update img's name"
}

function log_path_check()
{
    if [ ! -d ${ota_logpath}  ];then
        mkdir -p $ota_logpath
        touch $ota_info
        touch $progress
        touch $update_result
    fi
}

function partition_to_number()
{
    local partname=$1
    local i=0

    for element in uboot boot system
    do
        if [ x"$partname" = x"$element" ];then
            return $i
        fi
        i=$(($i+1))
    done
}


function printinfo()
{
    echo "[OTA_INFO] $1" >> $ota_info
}

function printprogress()
{
    echo "[OTA_INFO] $1" >> $progress
}

#check ota bin is exit
function check_ota_bin()
{
    if [ ! -f ${ota_tool} ];then
        echo "[OTA_INFO] Error: file ${ota_tool} not found!"
        exit 1
    fi
}

function get_ab_partition_status()
{
    ab_partition_status=$(${ota_tool} --partstatus -g)
    printinfo "get ab_partition_status $ab_partition_status"
}

function verify_rsa_signature()
{
    local file="${upgrade_file_path}"
    rm -rf $file
    mkdir -p $file
    local flag="true"
    local pubkey="/etc/pubkey.pem"
    local binary="${file}/binary.txt"

    unzip -q $1 -d ${file}
    local signature_file=${file}/signature.txt

    if [ -f $signature_file ];then
        tmp_file=$(cat $signatur_file | sed 's/\= /\=/g')
        for line in $tmp_file
        do
            local update_file=${line##*(}
            update_file=${update_file%\)*}  
            update_file=${file}/${update_file}
            local signature=${line##*=}

            if [ -f $update_file ];then
                for i in $(seq 0 2 ${#signature})
                do
                    s1=${signature:$i:2}
                    printf "\x${s1}"
                done > $binary

                # openssl verify signature
                result=$(openssl dgst -verify $pubkey -sha256 -signature $binary $update_file)
                if [ x"$result" = x"Verify OK" ];then
                    printinfo "Error: $update_file verify failed"
                    flag="false"
                    break
                fi
            else
                printinfo "Error: $update_file not exist"
                flag="false"
                break
            fi
        done    
    else
        printinfo "warning: file signature.txt not exist"
        flag="false"
    fi

    if [ x"$flag" = x"false" ];then
        printinfo "warning: rsa verify signature failed !"
 #       rm -rf $file
 #       exit 1
    else
        printinfo "rsa verify signature success !"
    fi
    printprogress "20%"
}

function check_sys_free_space()
{
    local userdata_free_space tmp_free_space

    # get /userdata and /tmp free space
    if [ x"$ota_bootmode" = x"emmc" ];then
        userdata_free_space=$(df -k /userdata | grep "userdata" | awk '{print $4}')
    else
        userdata_free_space=$(df -k /userdata | grep "ubi" | awk '{print $4}')
    fi
    tmp_free_space=$(df -k /tmp/ | grep "tmp" | awk '{print $4}')

    # get update file size
    val=$(unzip -l $image)
    val=${val##*------}
    upgrade_file_size=$(echo $val | awk '{print $1}')

    upgrade_need_space=$(($upgrade_file_size/1024))

    printinfo "upgrade_need_space: $upgrade_need_space"
    printinfo "tmp_free_space: $tmp_free_space"
    printinfo "userdata_free_space: $userdata_free_space"

    if [ $upgrade_need_space -gt $userdata_free_space ] && [ $upgrade_need_space -gt $tmp_free_space ];then
        printinfo "Error: not enough space in system"
        echo "2" >> $result
        exit 1
    elif [ $userdata_free_space -gt $upgrade_need_space ];then
        upgrade_file_path="/userdata/upgrade"
    elif [ $tmp_free_space -gt $upgrade_need_space ];then
        upgrade_file_path="/tmp"
    fi
    printprogress "10%"
}


function paramter_init()
{
    # get system partition status
    if [ x"$ota_upmode" = x"AB" ];then
        get_ab_partition_status
    fi
    printprogress "35%"
}

function update_flag()
{
    $ota_tool --updateflag -s $1
    local flag=$($ota_tool --updateflag -g)

    if [ x"$flag" = x"$1" ];then
        printinfo "update flag $flag success ! "
    else
        printinfo "Error: update flag $flag failed !"
        echo "2" >> $update_result
    fi
}

function ota_update_flag()
{
    # update flag
    update_flag

    # set resetreason
    local partition_tmp=${partition%_b*}
    printinfo "set resetreason $partition_tmp"
    if [ x"$partition_tmp" = x"all" ];then
        ab_partition_status=$(($ab_partition_status ^ 0x1E))
    else
        partition_to_number $partition_tmp
        local suffix=$?
        ab_partition_status=$(($ab_partition_status ^ (0x1 << $suffix)))
    fi
    ${ota_tool} --partstatus -s $ab_partition_status
    printinfo "update status $ab_partition_status finish"
}

function mtd_number_get()
{
    for element in /sys/class/mtd/*/; do
        if [ -f ${element}/name ] && [ "$(cat ${element}/name)" = "$1" ];then
            mtd_device=${element##/sys/class/mtd/}
            mtd_device="/dev/"${mtd_device%%/}
            return 0
        fi
    done
    printinfo "Error: $1 mtd device not exist !"
    echo "2" >> $update_result
    exit 1
}

function ubi_number_get()
{
    for element in /sys/class/ubi/*/; do
        if [ -f ${element}/name ] && [ "$(cat ${element}/name)" = "$1" ];then
            ubi_device=${element##/sys/class/ubi/}
            ubi_device="/dev/"${ubi_device%%/}
            return 0
        fi
    done
    printinfo "Error: $1 ubi device not exist !"
    echo "2" >> $update_result
    exit 1
}

function mmc_part_number()
{
    line=$(busybox fdisk -l | grep -w "$1")
    if [  ! -z "$line" ];then
        partition_id=$(echo "$line" | awk -F ' ' '{print $1}')
    else
        printinfo "Error: mmc part $1 not found"
        echo "2" >> $update_result
        exit 1
    fi
}


function update_nor_partition_img()
{
    local file=$1
    local partition=$2

    mtd_number_get $partition

    printinfo "flashcp -A -v $file ${mtd_device}"
    flashcp -A -v $file ${mtd_device}
    return $?
}

function prepare_tmp_rootfs()
{
    # remount rootfs, set read only
    mount -o remount,ro /

    mkdir -p ${minirootfs} 
    cd ${minirootfs}
    mkdir -p ${minirootfs}/usr/bin
    mkdir -p ${minirootfs}/usr/sbin
    mkdir -p ${minirootfs}/lib
    mkdir -p ${minirootfs}/sbin
    mkdir -p ${minirootfs}/bin
    mkdir -p ${minirootfs}/dev
    mkdir -p ${minirootfs}/proc
    mkdir -p ${minirootfs}/sys
    mkdir -p ${minirootfs}/tmp
    mkdir -p ${minirootfs}/userdata

    # cp file to minirootfs
    cp -rf /bin/busybox ${minirootfs}/bin/busybox
    cp -rf /bin/dd ${minirootfs}/bin/dd
    cp -rf /bin/sh ${minirootfs}/bin/sh
    cp -rf /bin/sh ${minirootfs}/bin/rm
    cp -rf /lib/libpthread-2.28.so ${minirootfs}/lib/libpthread.so.0
    cp -rf /lib/ld-2.28.so ${minirootfs}/lib/ld-linux-aarch64.so.1
    cp -rf /lib/libc-2.28.so ${minirootfs}/lib/libc.so.6
    cp -rf /lib/libm-2.28.so ${minirootfs}/lib/libm.so.6
    cp -rf /lib/libblkid.so.1.1.0 ${minirootfs}/lib/libblkid.so.1
    cp -rf /lib/libuuid.so.1.3.0 ${minirootfs}/lib/libuuid.so.1
    cp -rf /lib/libresolv-2.28.so ${minirootfs}/lib/libresolve.so.2
    cp -rf /sbin/reboot ${minirootfs}/sbin/rboot
    cp -rf /usr/bin/ota_tool ${minirootfs}/usr/bin/ota_tool
    cp -rf /usr/sbin/flashcp ${minirootfs}/usr/sbin/flashcp
    cp -rf /usr/sbin/flash_erase ${minirootfs}/usr/sbin/flash_erase
    cp -rf /usr/sbin/nandwrite.mtd-utils ${minirootfs}/usr/sbin/nandwrite
    cp -rf /bin/sync ${minirootfs}/bin/sync
    
    sync

    # mount nedd contents
    mount --bind /dev ${minirootfs}/dev
    mount --bind /proc ${minirootfs}/proc
    mount --bind /userdata ${minirootfs}/userdata
    mount --bind /tmp ${minirootfs}/tmp
    mount --bind /sys ${minirootfs}/sys
}

function upgrade_kernel()
{
    if [ ! -f ${upgrade_file_path}/vbmeta.img ] || [ ! -f ${upgrade_file_path}/boot.img ];then
        printinfo "Error: file vbmeta.img or boot.img not exist !"
        echo "2" >> $update_result
        exit 1
    fi

    if [ x"ota_bootmode" = x"nor_flash" ];then
        # get kernel mtd device
        mtd_number_get "vbmeta"

        printinfo "flashcp -v -A ${upgrade_file_path}/vbmeta.img ${mtd_device}"
        flashcp -v -A ${upgrade_file_path}/vbmeta.img ${mtd_device}
        ret1=$?

        # get kernel mtd device
        mtd_number_get "boot"

        printinfo "flashcp -v -A ${upgrade_file_path}/boot.img ${mtd_device}"
        flashcp -v -A ${upgrade_file_path}/boot.img ${mtd_device}
        ret2=$?
        
    elif [ x"ota_bootmode" = x"nand_flash" ];then
        # get kernel ubi volume
        ubi_number_get "vbmeta"

        printinfo "ubiupdatevol ${ubi_device} ${upgrade_file_path}/vbmeta.img "
        ubiupdatevol ${ubi_device} ${upgrade_file_path}/vbmeta.img
        ret1=$?

        # get kernel ubi volume
        ubi_number_get "boot"

        printinfo "ubiupdatevol ${ubi_device} ${upgrade_file_path}/boot.img "
        ubiupdatevol ${ubi_device} ${upgrade_file_path}/boot.img
        ret2=$?
    else
        mmc_part_number "boot"

        if [ x"$ota_upmode" = x"single" ];then
            vbmeta_id=$((${partition_id} - 1))
        elif [ x"$ota_upmode" = x"AB" ];then 
            vbmeta_id=$((${partition_id} - 2))
        else
            printinfo "Error: upmode ${ota_upmode} not support!"
            echo "2" >> $update_result
            exit 1
        fi

        dd if=${upgrade_file_path}/vbmeta.img of=/dev/mmcblk0p${vbmeta_id} bs=512 2> /dev/null
        ret1=$?

        dd if=${upgrade_file_path}/boot.img of=/dev/mmcblk0p${partition_id} bs=512 2> /dev/null
        ret2=$?
    fi

    if [ $ret1 != "0" ] || [ $ret2 != "0" ]; then
        printinfo "Errot: vbmeta/boot partition update failed !"
        echo "2" >> $update_result
        exit 1
    fi
}

function upgrade_uboot()
{
    if [ x"$ota_bootmode" = x"nor_flash" ];then
        #get mtd device 
        mtd_number_get "uboot"

        # erase and write uboot
        printinfo "flashcp -v -A ${upgrade_file_path}/uboot.img ${mtd_device}"
        flashcp -v -A ${upgrade_file_path}/uboot.img ${mtd_device}
        ret=$?
    elif [ x"$ota_bootmode" = x"nand_flash" ];then
        # get mtd device
        ubi_number_get "uboot" 
    else
        mmc_part_number "uboot"

        if [ x"${ota_upmode}" = x"single" ];then
            dd if=/dev/mmcblk0p${partition_id} of=${upgrade_file_path}/uboot.img bs=512 count=2048 seek=2048 2> /dev/null
            sync
        fi
        printinfo "dd if=${upgrade_file_path}/uboot.img of=/dev/mmcblk0p${partition_id} bs=512"
        dd if=${upgrade_file_path}/uboot.img of=/dev/mmcblk0p${partition_id} bs=512 2> /dev/null
        sync
        ret=$?
    fi
    if [ $? != "0" ];then
        printinfo "Error: write partition error"
        echo "2" >> $update_result
        exit 1
    fi
}

function upgrade_system()
{
    tmpinfo=$(df -h | grep "/dev/root")
    if [ -z "$tmpinfo" ];then
        board_status="recovery"
    fi

    if [ x"$ota_bootmode" = x"nor_flash" ];then
        mtd_number_get "system"

        prepare_tmp_rootfs

        printinfo "flashcp -v -A ${upgrade_file_path}/system.img ${mtd_device}"
        flashcp -v -A ${upgrade_file_path}/system.img ${mtd_device}
    elif [ x"$ota_bootmode" = x"nor_flash" ];then
        mtd_number_get "system"

        prepare_tmp_rootfs

        printinfo "flash_erase -q $mtd_device 0x0 "
        flash_erase -q $mtd_device 0x0 0
        printinfo "nandwrite -a -k -m -q -p $mtd_device ${upgrade_file_path}/system.img"
        nandwrite -a -k -m -q -p $mtd_device ${upgrade_file_path}/system.img
    elif [ x"$ota_upmode" = x"AB" ] || [ x"$board_status" = x"recovery" ];then
        mmc_part_number "system"
        printinfo "dd if=${upgrade_file_path}/system.img of=/dev/mmcblk0p${partition_id} bs=512"
        dd if=${upgrade_file_path}/system.img of=/dev/mmcblk0p${partition_id} bs=512 2> /dev/null
        ret=$?
        if [ $ret != "0" ];then
            printinfo "Error: system image write failed !"
            echo "2" >> $update_result
            exit 1
        fi
        sync
    elif [ x"$ota_upmode" = x"single" ];then
        prepare_tmp_rootfs
        mmc_part_number "system"
        
        printinfo "busybox dd if=${upgrade_file_path}/system.img of=/dev/mmcblk0p${partition_id} bs=512"
        busybox dd if=${upgrade_file_path}/system.img of=/dev/mmcblk0p${partition_id} bs=512 2> /dev/null 
        sync
    fi
}

function upgrade_all()
{
    # prepare for upgrade nor_flash
    if [ x"$ota_bootmode" = x"nor_flash" ];then
        echo "Error: nor does not support all_disk"
        echo "2" >> $update_file
        exit 1
    fi

    echo "prepare temp rootfs"
    prepare_tmp_rootfs

    local file_image=${upgrade_file_path}/$image

    echo "read eeprom valude"
    eeprom_value=$(${ota_tool} --eeprom -g)
    eeprom_value=${eeprom_vaule#*:}
    printprogress "upgrade disk.img"

    if [ x"$ota_bootmode" = x"nor_flash" ];then
        printinfo "falshcp -v -A ${file_image} ${mtd_device}"
        flashcp -v -A ${file_image} ${mtd_device}
    elif [ x"$ota_bootmod" = x"nand_flash" ];then
        printinfo "flash_erase ${mtd_device} 0x0 0"
        flash_erase $mtd_device 0x0 0
        printinfo "nandwrite -a -k -m ${mtd_device} ${file_image}"
        nandwrite -a -k -m ${mtd_device} ${file_image}
    else
        printinfo "dd if=${file_image} of=/dev/mmcblk0 bs=512"
        busybox dd if=${file_image} of=/dev/mmcblk0 bs=512 2> /dev/null
        sync
        printinfo "100%"
        busybox reboot -f
    fi

}


function update_img()
{
    local file=${upgrade_file_path}/tmpdir/update.sh
    local ret

    partition=${partition%%.zip}

    if [ x"$partition" = x"all" ];then
        upgrade_all
        ret=$?
    else
        if [ x"$ota_bootmode" = x"nor_flash" ];then
            # nor flash ota update
            # when find update.sh, progress update.sh
            part_id="NULL"
            if [ -f $file ];then
                chmod +x $file
                $file $part_id $ota_bootmode $upgrade_file_path
            else
                update_nor_partition_img $image $partition
            fi
            ret=$?
        else
            # emmc ota update
            part_id=$(mmc_part_number $partition)
            sync
            # when find update.sh, progress update.sh
            local upgrade_file=${upgrade_file_path}/${partition%_b*}.image
            if [ x"$partition" = x"uboot" ];then
                upgrade_uboot
            elif [ x"$partition" = x"boot" ];then
                upgrade_kernel
            elif [ x"$partition" = x"system" ];then
                upgrade_system
            else
                printinfo "Error: partition $partition not support !"
                echo "2" >> ${update_result}
                exit 1
            fi
            ret=$?
        fi
    fi
    
    if [ x"$ret" = x"0" ];then
        printinfo "update_img success!"
        printprogress "90%"
    else
        update_flag 13
        ret="1"
    fi
    sync
    return $ret
}


function all()
{
    # check log path
    log_path_check

    # check free space is enough
    check_sys_free_space 

    # verify signature
    verify_rsa_signature $image

    # init paramter: partition, file, name
    paramter_init
    if [ $? -ne 0 ];then
        printinfo "Error: ota update paramter check failed"
        return
    fi

    # set status befor upgrade
    ota_update_flag

    # write image
    update_img
    
}

partition=$1
image=$2
upgrade_file_path=''

ota_logpath="/userdata/cache"
progress="${ota_logpath}/progress"
ota_info="${ota_logpath}/info"
update_result="${ota_logpath}/result"

ota_tool="/usr/bin/ota_tool"

update_flag=0
ota_upmode="single"
ab_partition_status=0
reboot_flag=0
resetreason="normal"
ota_bootmode="emmc"
nor_upimg_finish="false"
mtd_device="/dev/mtd0"

minirootfs="/tmp/minirootfs"

mtd_device=''
ubi_device=''


#check utility
check_ota_bin

all

sync

