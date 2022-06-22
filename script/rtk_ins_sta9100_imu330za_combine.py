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
	
def fill_to_16(value):
    value_all = list(value)
    data_len_remain = len(value) % 16
    print(data_len_remain)
    if(data_len_remain != 0):
        for i in range(16 - data_len_remain):
            value_all.insert(len(value_all), 0xff)
    return bytes(value_all)

def file_combine(file_rtk,file_ins,file_sdk,file_imu,file_all):
    fs_rtk = open(file_rtk,'rb')
    fs_ins = open(file_ins,'rb')
    fs_sdk = open(file_sdk,'rb')
    fs_imu = open(file_imu,'rb')
    fs_all = open(file_all,'wb')

    rtk_data = fs_rtk.read()
    ins_data = fs_ins.read()
    sdk_data = fs_sdk.read()
    imu_data = fs_imu.read()
    
    sdk_data_fill = add_to_5120(sdk_data)
    rtk_data_fill = fill_to_16(rtk_data)
    ins_data_fill = fill_to_16(ins_data)
    imu_data_fill = fill_to_16(imu_data)
    
    rtk_size = len(rtk_data_fill)
    ins_size = len(ins_data_fill)
    sdk_size = len(sdk_data_fill)
    imu_size = len(imu_data_fill)
    
    rtk_size_list = get_list_from_int(rtk_size)
    ins_size_list = get_list_from_int(ins_size)
    sdk_size_list = get_list_from_int(sdk_size)
    imu_size_list = get_list_from_int(imu_size)
    

    fs_all.write(b'rtk_start:')
    fs_all.write(bytes(rtk_size_list))
    fs_all.write(rtk_data_fill)
    
    fs_all.write(b'ins_start:')
    fs_all.write(bytes(ins_size_list))
    fs_all.write(ins_data_fill)
    
    fs_all.write(b'sdk_start:')
    fs_all.write(bytes(sdk_size_list))
    fs_all.write(sdk_data_fill)	

    fs_all.write(b'imu_start:')
    fs_all.write(bytes(imu_size_list))
    fs_all.write(imu_data_fill)
    
if __name__ == '__main__':
	print(sys.argv[1])
	print(sys.argv[2])
	print(sys.argv[3])
	print(sys.argv[4])
	print(sys.argv[5])
	file_combine(sys.argv[1],sys.argv[2],sys.argv[3],sys.argv[4],sys.argv[5])

	