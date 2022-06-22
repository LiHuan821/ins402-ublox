import serial
import binascii
from time import sleep
import os 
import sys
import time

bin_file = 'IMU330LA.bin'
com_set = ''
if len(sys.argv) < 3:
    print ("set your com:")
    com_set = input()
    select = '1'
    #BAUD = '57600'
    BAUD = '115200'
    baud = int(BAUD)
elif len(sys.argv) == 4:
    com_set = sys.argv[1]
    baud = int(sys.argv[2])
    bin_file = sys.argv[3]

file_name = 'log.txt'
file = open(file_name,"wb")



#binfile = open("rtk.bin", 'rb')
#size = os.path.getsize("rtk.bin") 
print ("set your file:")

binfile = open(bin_file, 'rb')
size = os.path.getsize(bin_file) 
all_bytes_len = size
fs_len = size
print('size = %d' % size)
boot_mode = 0
print ("size:%d" % size)
upgrade_serial = serial.Serial(com_set, baud, timeout=0.1)  #/dev/ttyUSB0

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

def get_list_from_int(value):
    value_list = [0 for x in range(0, 4)]
    value_list[3] = value & 0xff
    value_list[2] = (value >> 8) & 0xff
    value_list[1] = (value >> 16) & 0xff
    value_list[0] = (value >> 24) & 0xff
    return value_list


def start_bootloader():
    '''Starts bootloader
        :returns:
            True if bootloader mode entered, False if failed
    '''
    print ("start")
    C = [0x55, 0x55, ord('J'), ord('B'), 0x00 ]
    crc = calc_crc(C[2:4] + [0x00])    # for some reason must add a payload byte to get correct CRC
    crc_msb = (crc & 0xFF00) >> 8
    crc_lsb = (crc & 0x00FF)
    C.insert(len(C), crc_msb)
    C.insert(len(C), crc_lsb)
    upgrade_serial.write([0x01,0x02])
    sleep(2)
    upgrade_serial.write(C)
    #print (C)
    time.sleep(2)   # must wait for boot loader to be ready
    R = upgrade_serial.read_all()
    '''
    if R[0] == 85 and R[1] == 85:
        #packet_type =  '{0:1c}'.format(R[2]) + '{0:1c}'.format(R[3])
        packet_type = R[2] + R[3]
        if packet_type == 'JB':
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
    '''
    return True

def set_core(upgrade_serial,size,core):
    size_list = get_list_from_int(size)
    C = [0x55, 0x55, ord('C'), ord('S'),6, ord('C'), ord(core) ]
    C = C + size_list
    crc = calc_crc(C[2:])
    crc_msb = (crc & 0xFF00) >> 8
    crc_lsb = (crc & 0x00FF)
    C.insert(len(C), crc_msb)
    C.insert(len(C), crc_lsb)
    upgrade_serial.write(C)
    sleep(2)

    R = upgrade_serial.read_all()   #7
    if R[0] == 85 and R[1] == 85:
        packet_type = '{0:1c}'.format(R[2]) + '{0:1c}'.format(R[3])
        print('type:',packet_type)

def start_app():
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
        print('type:',packet_type)

def write_block(buf, data_len, addr):
    '''Executed WA command to write a block of new app code into memory
    '''
    #print(data_len, addr)
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
    #print(data_len_remain)
    data_len_ori = data_len
    if(data_len_remain != 0):
        data_len = data_len + (16 - data_len_remain)
        print('------------------------------------ = %d' % data_len_remain)
        print('------------------------------------ = %d' % data_len)
    C.insert(len(C), data_len)
    for i in range(data_len_ori):
        #C.insert(len(C), ord(buf[i]))
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
        #print(C)
        upgrade_serial.write(C)
        #input('wait')
        test = []
        for ele in C:
            test.insert(len(test),hex(ele))
        print ("upload progress: %.3f%%" % (float(addr)/float(fs_len)*100))
        if addr == 0:
           sleep(15)
        else:
           #sleep(0.01)
           sleep(0.02)
        print('block wait')
        #input()
        R = upgrade_serial.read(12)  #longer response
        response=[]
        for ele in bytearray(R):
            response.append(ele)
        print('respose:',response)
        #test = ord(R[0])
        status = 1
        if len(R) > 1 and (R[0]) == 85 and (R[1]) == 85:
            #packet_type =  '{0:1c}'.format(R[2]) + '{0:1c}'.format(R[3])
            #packet_type = R[2] + R[3]
            #print(packet_type)
            packet_type = '{0:1c}'.format(
                R[2]) + '{0:1c}'.format(R[3])
            if packet_type == 'WA':
                status = 1
            else:
                #sys.exit()
                input('continue')
                print('retry 1')
                status = 0
        else:
            print('R = ',R)
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
		'''
		if data == '':
			continue
		else:
			break
		'''
		#sleep(0.02)
	return data

write_flag = 1
send_count = 0
max_data_len = 192
write_len = 0
#start_app()
if __name__ == '__main__':

    if upgrade_serial.isOpen() :
        print("open success")
        start_time = time.time()
    else :
        print("open failed")
    #while True:
    sleep(1)
    set_core(upgrade_serial,0,'2')
    #input('wait')
    sleep(3)
    upgrade_serial.close()
    upgrade_serial = serial.Serial(com_set, baud, timeout=0.1)  #/dev/ttyUSB0
    start_bootloader()

    '''
    while True:
        sleep(1)
        print('wait')
    '''
    sleep(5)
    '''
    upgrade_serial.reset_input_buffer()
    upgrade_serial.close()
    upgrade_serial.baud = 115200
    upgrade_serial.open()
    sleep(2)
    '''
    upgrade_serial.close()
    upgrade_serial = serial.Serial(com_set, baud, timeout=0.1)  #/dev/ttyUSB0

    while (write_len < fs_len):
        data_to_write = binfile.read(max_data_len)
        #packet_data_len = max_data_len 
        if (fs_len - write_len) > max_data_len:
            packet_data_len = max_data_len 
        else:
            packet_data_len = fs_len - write_len
        #print(data_to_write)
        write_block(data_to_write,packet_data_len, write_len)
        write_len += packet_data_len
    print ("upload progress: %.3f%%" % 100)
    end_time = time.time()
    print ("upload use time: %.2fs" % (end_time - start_time))
    print ("start app")
    sleep(5)
    start_app()
    '''
    while write_flag > 0:
        data_to_write = binfile.read(100)
        upgrade_serial.write(data_to_write)
        all_bytes_len -= 100
        send_count += 100
        if(all_bytes_len) < 100:
            data_to_write = binfile.read(all_bytes_len - 1)
            sleep(0.08)
            upgrade_serial.write(data_to_write)
            write_flag = 0
            print send_count
            break
        sleep(0.08)
    '''
    print ('end')
        #upgrade_serial.write(data) 
