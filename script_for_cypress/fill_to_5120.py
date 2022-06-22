import os
import time
import sys


def add_to_5120(data):
    print(len(data))
    data_to_add_len = 5120 - len(data) % 5120
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
	
def file_combine(file_sdk,file_all):
    fs_sdk = open(file_sdk,'rb')
    fs_all = open(file_all,'wb')
    sdk_data = fs_sdk.read()
    if (len(sdk_data) > 0):
        sdk_data_fill = add_to_5120(sdk_data)
    sdk_size = len(sdk_data_fill)

    fs_all.write(sdk_data_fill)	
	
if __name__ == '__main__':
	print(sys.argv[1])
	print(sys.argv[2])
	file_combine(sys.argv[1],sys.argv[2])

	

	
	