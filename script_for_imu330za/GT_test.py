import os
import time
import serial
import _thread
import datetime  
import sys  
  
#gt_data_list = [0x55, 0x55, 0x47, 0x54, 0x06, 0xf8, 0x0d, 0x84, 0x11, 0x76, 0x08, 0xa2, 0xb8, 0x55, 0x55, 0x47, 0x54, 0x06, 0xe0, 0x11, 0x84, 0x11, 0x76, 0x08, 0x3a, 0x0f]


#gt_data_list = [0x55, 0x55, 0x47, 0x54, 0x06, 0xf8, 0x0d, 0x84, 0x11, 0x76, 0x08, 0xa2, 0xb8 ]
gt_data_list = [0x55, 0x55, 0x47, 0x54, 0x06, 0x11, 0x84, 0x0D, 0xF8, 0x76, 0x08, 0xFB, 0x21 ]


def calc_crc(payload):
    '''
    Calculates 16-bit CRC-CCITT
    '''
    crc = 0x1D0F
    for bytedata in payload:
        crc = crc ^ (bytedata << 8)
        i = 0
        while i < 8:
            if crc & 0x8000:
                crc = (crc << 1) ^ 0x1021
            else:
                crc = crc << 1
            i += 1

    crc = crc & 0xffff
    crc_msb = (crc & 0xFF00) >> 8
    crc_lsb = (crc & 0x00FF)
    return [crc_msb, crc_lsb]

'''
crc_get = calc_crc([0x47, 0x54, 0x06, 0x11, 0x84, 0x0D, 0xF8, 0x76, 0x08])
print(crc_get)
exit(0)
'''

def get_utc_day():
	year = int(time.strftime("%Y"))
	month = int(time.strftime("%m"))
	day = int(time.strftime("%d"))
	hour = int(time.strftime("%H"))
	minute = int(time.strftime("%M"))
	second = int(time.strftime("%S"))
	local_time = datetime.datetime(year, month, day, hour, minute, second)
	time_struct = time.mktime(local_time.timetuple())
	utc_st = datetime.datetime.utcfromtimestamp(time_struct)
	d1 = datetime.datetime(year, 1, 1)
	utc_sub = utc_st - d1
	utc_str = utc_sub.__str__()
	utc_day_int = int(utc_str.split( )[0])
	utc_day_str = str(utc_day_int + 1)
	return utc_day_str

def rev_s2_data(gt_serial,is_log):
    if is_log:
        day = get_utc_day()
        file_time = time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime())
        log_file = 'user_' + file_time + '_S2' +'.bin'
        fs = open(log_file,'wb')
    length = 0
    while True:
        rev_data = gt_serial.read_all()
        if(len(rev_data) > 0):
            length+= len(rev_data)
            sys.stdout.write("\rread length: %d" %(length))
            sys.stdout.flush()
            if is_log:
                fs.write(rev_data)
        time.sleep(0.1)
        

if __name__ == '__main__': 
    gt_serial = serial.Serial('com9', 230400, timeout=0.005)
    _thread.start_new_thread(rev_s2_data,(gt_serial,1,))
    while True:
        gt_serial.write(bytes(gt_data_list))
        time.sleep(1)
