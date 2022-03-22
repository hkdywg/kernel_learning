#!/usr/bin/python

import os

output_dir = "output"

def file_trans(filename):
    # 打开一个文件
    file_read = open(filename, "rb")
    file_name_r = os.path.splitext(filename)[0]
    #print(file_name_r)
    file_name_r_suffix = os.path.splitext(filename)[-1]
    if file_name_r_suffix == ".c" :
        file_name_w = output_dir + "/" + file_name_r + ".cc"
    elif file_name_r_suffix == ".h":
        file_name_w = output_dir + "/" + file_name_r + ".hh"
    elif file_name_r_suffix == ".cc":
        file_name_w = output_dir + "/" + file_name_r + ".c"
    elif file_name_r_suffix == ".hh":
        file_name_w = output_dir + "/" + file_name_r + ".h"
    else:
        return
    str = file_read.read()
    file_write = open(file_name_w, "wb")
    file_write.write(str)
    file_read.close()
    file_write.close()
    return


def list_filepath(file_path):
    files = os.listdir(file_path)
    for file in files:
        file_d = os.path.join(file_path, file)
        if os.path.isdir(file_d):
            if file == output_dir:
                print("ignore dir")
            else:
                #print("ignore dir--")
                os.mkdir(output_dir + "/" + file_d)
                list_filepath(file_d)
        else:
            file_trans(file_d)
    return


if os.path.exists(output_dir):
    print("outdir is exit")
    list_filepath(".")
else:
    os.mkdir(output_dir)
    list_filepath(".")

