import os
import time
import sys


def fill_rtk_data(data):
    print(len(data))
    all_len = 0x10280000 - 0x10080000
    data_to_add_len = all_len - len(data) % all_len
    print(data_to_add_len)
    data_to_add = []
    for i in range(data_to_add_len):
        data_to_add.append(0xff)
    data_all = data + bytes(data_to_add)
    print(len(data_all))
    return data_all


def get_list_from_int(value):
	value_list = [0 for x in range(0, 4)]
	value_list[0] = value & 0xff
	value_list[1] = (value >> 8) & 0xff
	value_list[2] = (value >> 16) & 0xff
	value_list[3] = (value >> 24) & 0xff
	return value_list
	
def file_combine(file_rtk,file_ins,file_all):
    fs_rtk = open(file_rtk,'rb')
    fs_ins = open(file_ins,'rb')
    fs_all = open(file_all,'wb')
    rtk_size = os.path.getsize(file_rtk)
    rtk_data = fs_rtk.read()
    rtk_data = fill_rtk_data(rtk_data)
    
    ins_size = os.path.getsize(file_ins)
    ins_data = fs_ins.read()

    fs_all.write(rtk_data)
    fs_all.write(ins_data)	
	
if __name__ == '__main__':
	print(sys.argv[1])
	print(sys.argv[2])
	print(sys.argv[3])
	file_combine(sys.argv[1],sys.argv[2],sys.argv[3])

	

	
	