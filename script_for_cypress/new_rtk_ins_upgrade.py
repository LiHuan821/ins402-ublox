import serial
import binascii
from time import sleep
import os 
import sys
import time


file_name = 'log.txt'
file = open(file_name,"wb")
boot_mode = 0
select = '1'
BAUD = '115200'
baud = int(BAUD)
print ("set your com:")
com_set = input()

'''
bin_file = input('upgrade file:  ')
binfile = open(bin_file, 'rb')
core = input('core: ')
size = os.path.getsize(bin_file) 
fs_len = size
'''
fs_len = 0

def get_list_from_int(value):
    value_list = [0 for x in range(0, 4)]
    value_list[3] = value & 0xff
    value_list[2] = (value >> 8) & 0xff
    value_list[1] = (value >> 16) & 0xff
    value_list[0] = (value >> 24) & 0xff
    return value_list


def calc_crc(payload):
	'''Calculates CRC per 380 manual
	'''
	crc = 0x1D0F
	for bytedata in payload:
		crc = crc^(bytedata << 8) 
		for i in range(0,8):
			if crc&0x8000:
				crc = (crc << 1)^0x1021
			else:
				crc = crc << 1

	crc = crc & 0xffff
	return crc

def start_bootloader(upgrade_serial):
    '''Starts bootloader
        :returns:
            True if bootloader mode entered, False if failed
    '''
    print ("start")
    C = [0x55, 0x55, ord('J'), ord('I'), 0x00 ]
    crc = calc_crc(C[2:4] + [0x00])    # for some reason must add a payload byte to get correct CRC
    crc_msb = (crc & 0xFF00) >> 8
    crc_lsb = (crc & 0x00FF)
    C.insert(len(C), crc_msb)
    C.insert(len(C), crc_lsb)
    #upgrade_serial.write([0x01,0x02])
    #sleep(2)
    upgrade_serial.write(C)
    time.sleep(4)   # must wait for boot loader to be ready
    R = upgrade_serial.read_all()
    if R[0] == 85 and R[1] == 85:
        #packet_type =  '{0:1c}'.format(R[2]) + '{0:1c}'.format(R[3])
        packet_type = R[2] + R[3]
        if packet_type == 'JI':
            upgrade_serial.read(R[4]+2)
            print('bootloader ready')
            time.sleep(2)
            if boot_mode == 0:
                print('resync with device')
                time.sleep(2)
            return True
        else: 
            return False
    else:
        return False

def set_core(upgrade_serial,size,core):
    size_list = get_list_from_int(size)
    C = [0x55, 0x55, ord('C'), ord('S'),6, ord('C'), ord(core) ]
    C = C + size_list
    crc = calc_crc(C[2:])    # for some reason must add a payload byte to get correct CRC
    crc_msb = (crc & 0xFF00) >> 8
    crc_lsb = (crc & 0x00FF)
    C.insert(len(C), crc_msb)
    C.insert(len(C), crc_lsb)
    upgrade_serial.write(C)
    sleep(2)

    R = upgrade_serial.read_all()   #7
    if R[0] == 85 and R[1] == 85:
        packet_type = '{0:1c}'.format(R[2]) + '{0:1c}'.format(R[3])
        print(packet_type)


def start_app(upgrade_serial):
    '''Starts app
    '''
    C = [0x55, 0x55, ord('J'), ord('A'), 0x00 ]
    crc = calc_crc(C[2:4] + [0x00])    # for some reason must add a payload byte to get correct CRC
    crc_msb = (crc & 0xFF00) >> 8
    crc_lsb = (crc & 0x00FF)
    C.insert(len(C), crc_msb)
    C.insert(len(C), crc_lsb)
    upgrade_serial.write(C)
    sleep(2)
    R = upgrade_serial.read_all()   #7
    '''
    if (R[0]) == 85 and (R[1]) == 85:
        packet_type =  R[2] + R[3]
        print(packet_type)
    '''
    if R[0] == 85 and R[1] == 85:
        packet_type = '{0:1c}'.format(R[2]) + '{0:1c}'.format(R[3])
        print(packet_type)
        return True
    else:
        return False

def write_block(buf, data_len, addr,upgrade_serial):
    '''Executed WA command to write a block of new app code into memory
    '''
    global fs_len
    print(data_len, addr)
    C = [0x55, 0x55, ord('W'), ord('A'), data_len+5]
    addr_3 = (addr & 0xFF000000) >> 24
    addr_2 = (addr & 0x00FF0000) >> 16
    addr_1 = (addr & 0x0000FF00) >> 8
    addr_0 = (addr & 0x000000FF)
    C.insert(len(C), addr_3)
    C.insert(len(C), addr_2)
    C.insert(len(C), addr_1)
    C.insert(len(C), addr_0)
    data_len_remain = data_len % 16
    data_len_ori = data_len
    if(data_len_remain != 0):
        data_len = data_len + (16 - data_len_remain)
        '''
        print('------------------------------------ = %d' % data_len_remain)
        print('------------------------------------ = %d' % data_len)
        '''
    C.insert(len(C), data_len)
    for i in range(data_len_ori):
        C.insert(len(C), buf[i])
    if(data_len_remain != 0):
        C[4] = data_len + 5
        for i in range(16 - data_len_remain):
            C.insert(len(C), 0)
    crc = calc_crc(C[2:C[4]+5])  
    crc_msb = int((crc & 0xFF00) >> 8)
    crc_lsb = int((crc & 0x00FF))
    C.insert(len(C), crc_msb)
    C.insert(len(C), crc_lsb)
    status = 0
    while (status == 0):
        upgrade_serial.write(C)
        #input('wait\r\n')
        test = []
        for ele in C:
            test.insert(len(test),hex(ele))
        print ("upload progress: %.3f%%" % (float(addr)/float(fs_len)*100))
        if addr == 0:
            print('sleep start')
            sleep(15)
        else:
            #sleep(0.01)
            sleep(0.01)
        #input()
        R = upgrade_serial.read(12)  #longer response
        response=[]
        for ele in bytearray(R):
            response.append(ele)
        #test = ord(R[0])
        status = 1
        if len(R) > 1 and (R[0]) == 85 and (R[1]) == 85:
            packet_type = '{0:1c}'.format(
                R[2]) + '{0:1c}'.format(R[3])
            if packet_type == 'WA':
                status = 1
            else:
                #sys.exit()
                input('continue')
                status = 0
        else:
            #self.reset_buffer()
            sleep(1)
            print('no packet')
            #sys.exit()
        


def recv(upgrade_serial):
	while True:
		if select == "2":
			data = binascii.b2a_hex(upgrade_serial.read_all())
			file.write(data)
			#data = upgrade_serial.read_all()
			#data = "test"
		else:
			#data = binascii.b2a_hex(upgrade_serial.read_all())
			data = upgrade_serial.read_all()
			file.write(data)
		#sleep(0.02)
	return data

max_data_len = 192




def upgrade_rtk():
    global fs_len
    write_len = 0
    bin_file = 'rtk.bin'
    binfile = open(bin_file, 'rb')
    core = '0'
    size = os.path.getsize(bin_file) 
    fs_len = size
    
    upgrade_serial = serial.Serial(com_set, baud, timeout=0.1)
    if upgrade_serial.isOpen() :
        print("open success")
        start_time = time.time()
    else :
        print("open failed")
    sleep(1)
    start_bootloader(upgrade_serial)
    sleep(6)
    upgrade_serial.reset_input_buffer()
    upgrade_serial.close()
    time.sleep(2)
    #upgrade_serial.open()
    upgrade_serial = serial.Serial(com_set, '115200', timeout=0.1)  #/dev/ttyUSB0

    time.sleep(2)
    set_core(upgrade_serial,size,core)
    while (write_len < fs_len):
        data_to_write = binfile.read(max_data_len)
        #packet_data_len = max_data_len 
        if (fs_len - write_len) > max_data_len:
            packet_data_len = max_data_len 
        else:
            packet_data_len = fs_len - write_len
        #print(data_to_write)
        write_block(data_to_write,packet_data_len, write_len,upgrade_serial)
        write_len += packet_data_len
    print ("upload progress: %.3f%%" % 100)
    end_time = time.time()
    print ("upload use time: %.2fs" % (end_time - start_time))
    print ("start app")
    sleep(4)
    upgrade_state = start_app(upgrade_serial)
    sleep(6)
    if upgrade_state == True:
        print ('end')
        upgrade_serial.close()
        return True
    else:
        upgrade_serial.close()
        return False

def upgrade_ins():
    global fs_len
    write_len = 0
    bin_file = 'ins.bin'
    binfile = open(bin_file, 'rb')
    core = '1'
    size = os.path.getsize(bin_file) 
    fs_len = size
    
    upgrade_serial = serial.Serial(com_set, baud, timeout=0.1)
    if upgrade_serial.isOpen() :
        print("open success")
        start_time = time.time()
    else :
        print("open failed")
    sleep(1)
    start_bootloader(upgrade_serial)
    sleep(6)
    upgrade_serial.reset_input_buffer()
    upgrade_serial.close()
    time.sleep(2)
    #upgrade_serial.open()
    upgrade_serial = serial.Serial(com_set, '115200', timeout=0.1)  #/dev/ttyUSB0

    time.sleep(2)
    set_core(upgrade_serial,size,core)
    while (write_len < fs_len):
        data_to_write = binfile.read(max_data_len)
        #packet_data_len = max_data_len 
        if (fs_len - write_len) > max_data_len:
            packet_data_len = max_data_len 
        else:
            packet_data_len = fs_len - write_len
        write_block(data_to_write,packet_data_len, write_len,upgrade_serial)
        write_len += packet_data_len
    print ("upload progress: %.3f%%" % 100)
    end_time = time.time()
    print ("upload use time: %.2fs" % (end_time - start_time))
    print ("start app")
    sleep(4)
    upgrade_state = start_app(upgrade_serial)
    sleep(6)
    if upgrade_state == True:
        print ('end')
        upgrade_serial.close()
        return True
    else:
        upgrade_serial.close()
        return False
all_count = 0
if __name__ == '__main__':	
    while True:
        state = upgrade_rtk()
        print('upgrade count: {0}'.format(all_count))
        if state == True:
            print('suc')
        else:
            print('fail')
            exit(-1)    
        state = upgrade_ins()
        print('upgrade count: {0}'.format(all_count))
        if state == True:
            print('suc')
        else:
            print('fail')
            exit(-1)       
        all_count+= 1
        print('upgrade count: {0}'.format(all_count))
        exit(0)

