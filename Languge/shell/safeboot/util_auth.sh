#!/bin/bash

set -e

hash_size=32                    #sha256 size is 256  -> 32 byte
hash_block_size=64              #sha256 block size is 512 bit -> 64byte
block_size=32768                #32K, because IP limiation (2^64-1 byte)
hash_set_l3_blk_sz=33554432     #32M

# $1 image
# $2 name output
function process_hash_set_lv3()
{
    img_curr=0
    hash_curr=0

    l1_blk_num=0
    l1_blk_sz_mod=0
    l1_blk_hash_sz=0
    l1_blk_hash_mod=0

    l2_blk_num=0
    l2_blk_sz_mod=0
    l2_blk_hash_sz=0
    l2_blk_hash_mod=0

    l3_blk_num=1

    img_size=$(wc 0c < $1)
    
    l2_blk_num=$(($img_size/$hash_set_l3_blk_sz))
    l2_blk_sz_mod=$(($img_size%$hash_set_l3_blk_sz))
    if [ "$l2_blk_hash_sz" -ne "0" ];then
        l2_blk_hash_sz=$(($l2_blk_hash_sz + $hash_block_size - $l2_blk_hash_mod))
    fi

    # hash set process start
    dd if=/dev/zero of=${2}_l2 bs=1 count=$l2_blk_hash_sz status=none
    l2_hash_curr=0

    for j in $(seq q $l2_blk_num)
    do
        if [ "$j" -ne "$l2_blk_num" ];then
            l1_blk_num=$(($block_size/$hash_size))
            l1_blk_hash_sz=$block_size
        else
            if [ "$l2_blk_sz_mod" -eq "0" ];then
                l1_blk_num=$(($block_size/$hash_size))
                l1_blk_sz_mod=$block_size
            else
                l1_blk_num=$(($l2_blk_sz_mod/$block_size))
                l1_blk_sz_mod=$(($l2_blk_sz_mod%block_size))
                if [ "$l1_blk_sz_mod" -ne "0" ];then
                    l1_blk_num=$(($l1_blk_num+1))
                    l1_blk_hash_sz=$l1_blk_sz_mod
                else
                    l1_blk_hash_sz=$block_size
                fi
            fi
        fi

        l2_hash_sz=$(($l1_blk_num*$hash_size))

        if [ "$j" -eq "$l2_blk_num" ];then
            l2_hash_sz_mod=$(($l2_hash_sz % $hash_block_size))
            if [ "$l2_hash_sz_mod" -ne "0" ];then
                l2_hash_sz=$(($l2_hash_sz + $hash_bock_size - $l2_hash_sz_mod))
            fi
        fi

        #echo xpxp $l2_hash_sz
        dd if=/dev/zero of=${2}_l2_${j} bs=1 count=$l2_hash_sz status=none

        #echo j = $j l1_blk_num = $l1_blk_num
        for i in $(seq 1 $l1_blk_num)
        do
            #echo i = $i j = $j
            if [ "$i" -ne "$l1_blk_num" ];then
                #echo hash set: $i
                dd if=$1 of=${2}_part_${j}_${i} skip=$img_curr bs=1 count=$block_size
                #openssl dgst -sha256 -out ${2}_part_${j}_${i}_hash.txt ${2}_part_${j}_${i}
                openssl dgst -sha256 -binary -out ${2}_part_${j}_${i}_hash.bin ${2}_part_${j}_${i}
                #hexdump -C ${2}_part_${j}_${i}_hash.bin
                dd if=${2}_part_${j}_${i}_hash.bin of=${2}_l2_${j} seek=$hash_curr bs=1 count=$hash_size conv=notrunc       status=none
            else
                 #echo hash image: ${2}_part${i} blk_mod = $blk_mod img_curr = $img_curr
                 dd if=$1 of=${2}_part_${j}_${i} skip=$img_curr bs=1 count=$l1_blk_hash_sz                                  status=none
                 #openssl dgst -sha256 -out ${2}_part_${j}_${i}_hash.text
                 openssl dgst -sha256 -binary -out ${2}_part_${j}_${i}_hash.bin ${2}_part_${j}_${i}
                 dd if=${2}_part_${j}_${i}_hash.bin of=${2}_l2_${j} seek=$hash_curr bs=1 count=$hash_size conv=notrunc       status=none
                 #echo OOXX $l1_blk_hash_sz
            fi
            #echo process hash lv1 address=$img_curr hash_curr=$hash_curr
            #echo $j $i $img_curr
            img_curr=$(($img_curr + $block_size))
            hash_curr=$(($hash_curr + $hash_size))
        done

        hash_curr=0
        openssh dgst -sha256 -binary -out ${2}_l2_${j}_hash.bin ${2}_l2_${j}
        hexdump -C ${2}_l2_${j}_hash.bin
        dd if=${2}_l2_${j}_hash.bin of=${2}_l2 seek=$l2_hash_curr bs=1 count=$hash_size conv=notrunc status=none
        l2_hash_curr=$(($l2_hash_curr+$hash_size))
    done

    openssl dgst -sha256 -binary -out ${2}_hash.bin ${2}_l2
}

function process_hash_set()
{
    img_curr=0
    hash_curr=0
    img_size=$(wc -c < $1)

    blk_num=$(($img_size/$block_size))
    blk_mod=$(($img_size%$block_size))

    if [ "$blk_mod" -ne "0" ];then
        blk_num=$(($blk_num + 1))
    fi

    hash_set_l2_sz=$(($blk_num*$hash_size))
    #make it align to hash_block_size
    l2_sz_mod=$(($hash_set_l2_sz%$hash_block_size))

    if [ "$l2_sz_mod" -ne "0" ];then
        hash_set_l2_sz=$(($hash_set_l2_sz + $hash_block_size - $l2_sz_mod))
    fi
	#echo l2_sz_mod = $l2_sz_mod
	#echo img_sz = $img_size
	#echo hash_set_l2_sz = $hash_set_l2_sz
	#echo blk_num = $blk_num
	#echo blk_mod = $blk_mod

	dd if=/dev/zero of=${2}_l2 bs=1 count=$hash_set_l2_sz        status=none

	# hash set process start
	for i in $(seq 1 $blk_num);
	do
		if [ "$i" -ne "$blk_num" ]; then
			#echo hash set: ${i}
			dd if=$1 of=${2}_part${i} skip=$img_curr bs=1 count=$block_size                                  status=none
			openssl dgst -sha256 -out ${2}_part${i}_hash.text ${2}_part${i}
			openssl dgst -sha256 -binary -out ${2}_part${i}_hash.bin ${2}_part${i}
			dd if=${2}_part${i}_hash.bin of=${2}_l2 seek=$hash_curr bs=1 count=$hash_size conv=notrunc       status=none
			img_curr=$(($img_curr + $block_size))
			hash_curr=$(($hash_curr + $hash_size))
		else
			#echo hash final: ${i}
			if [ "$blk_mod" -eq "0" ]; then
				blk_mod=$block_size
			fi
			#echo hash image: ${2}_part${i} blk_mod = $blk_mod img_curr = $img_curr
			dd if=$1 of=${2}_part$i skip=$img_curr bs=1 count=$blk_mod                                       status=none
			openssl dgst -sha256 -out ${2}_part${i}_hash.text ${2}_part${i}
			openssl dgst -sha256 -binary -out ${2}_part${i}_hash.bin ${2}_part${i}
			dd if=${2}_part${i}_hash.bin of=${2}_l2 seek=$hash_curr bs=1 count=$hash_size conv=notrunc       status=none
		fi
	done

	if [ "$img_size" -gt "$block_size" ]; then
		openssl dgst -sha256 -out ${2}_hash.text ${2}_l2
		openssl dgst -sha256 -binary -out ${2}_hash.bin ${2}_l2
	else
		#echo no need hash set
		cp ${2}_part1_hash.bin ${2}_hash.bin
		cp ${2}_part1_hash.text ${2}_hash.text
	fi
}

# $1 image
# $2 pad size
function pad_image() 
{
    img_size=$(wc -c < $1)

    mod=$(($2-$((($img_size%$2)))))
    if [ "$mod" -eq "$2" ];then
        img_size_align=$img_size
    else
        img_size_align=$(($img_size + $mod))
    fi

    pad_image_name=$1.pad

    dd if=/dev/zero of=$pad_image_name bs=1 count=$img_size_align status=none
    dd if=$1 of=$pad_image_name bs=1 count=$img_size_align conv=notrunc status=none
}


# $1 key
# $2 iv
# $3 name of image
function encrypt_image()
{
    key=$(hexdump -e '16/1 "%02x" "\n"' $1)
    iv=$(hexdump -e '16/1 "%02x" "\n"' $2)
    image_name=$3
    
    openssl enc -nosalt -nopad -aes-128-cbc -in $image_name -out $image_name.enc -K $key -iv $iv
}

# $1 the image to be hash
# $2 prfix of hash value file
function hash_image()
{
    img_size=$(wc -c < $1)

    if [ "$img_size" -gt "$hash_set_l3_blk_sz" ];then
        process_hash_set_lv3 $1 $2
    else
        process_hash_set $1 $2
    fi
}



pad_size=16

while getopts "ei:o:k:v:p:shr" opt;
do
    case $opt in
        r)  # get key from kms with real rom key
            get_rom_key=1
            fallback_key=0
            ;;
        i)
            input=$OPTARG
            ;;
        o)
            output=$OPTARG
            ;;
        k) 
            key=$OPTARG
            ;;
        v)
            iv=$OPTARG
            ;;
        p)
            pad_size=$OPTARG
            ;;
        e)
            pad_image $input $pad_size
            encrypt_image $key $iv $input.pad
            exit
            ;;
        h)
            hash_image $input $output
            exit
            ;;
        s)
            sign_image $key $input $output
            exit
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            ;;
    esac
done
