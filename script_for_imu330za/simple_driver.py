#!/usr/bin/python
#!coding:utf-8

from socket import *
import os,sys
import time
import psutil
import serial
import sys

if len(sys.argv) < 3:
    print('para wrong')
com_set = sys.argv[1]

baud_set = sys.argv[2]


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


def packet_cmd(cmd):
	packet = []
	packet.extend(bytes(cmd, 'utf-8'))
	msg_len = 0
	packet.append(msg_len)
	final_packet = packet
	cmd_packet = [0x55,0x55] + final_packet + calc_crc(final_packet)
	return bytes(cmd_packet)



def data_parse(data,cmd):
    parse_state = 0
    get_data = []
    len = 0
    get_len = 0
    for ele in data:
        if parse_state == 0:
            if ele == 0x55:
                parse_state = 1
                continue
        if parse_state == 1:
            if ele == 0x55:
                parse_state = 2
                continue
        if parse_state == 2:
            if ele == ord(cmd[0]):
                parse_state = 3
                continue
            else:
                parse_state = 0
                continue
        if parse_state == 3:
            if ele == ord(cmd[1]):
                parse_state = 4
                print('--------------------------')
                continue
            else:
                parse_state = 0
                continue
                
        if parse_state == 4:
            len = ele
            parse_state = 5
            continue
        if parse_state == 5:
            if get_len < len:
                get_len+= 1
                get_data.append(ele)
                continue
            else:
                return get_data
                continue

crc_check = [0,0]

data_len = 0

def data_parse(data,cmd):
    data_to_check_crc = []
    parse_state = 0
    data_get = [0,0,0]
    get_len = 0

    for ele in data:
        if parse_state == 0:
            if ele == 85:
                parse_state = 1
                continue
        if parse_state == 1:
            if ele == 85:
                parse_state = 2
                continue
            else:
                parse_state = 0
                continue
        if parse_state == 2:
            get_packet_flag = False

            if ele == ord(cmd[0]):
                parse_state = 3
                data_get[0] = ele
                data_to_check_crc.append(ele)
                get_packet_flag = True
            if get_packet_flag == False:
                parse_state = 0
                data_to_check_crc = []
            else:
                parse_state = 3
            continue
        if parse_state == 3:
            get_packet_flag = False
            if ele == ord(cmd[1]):
                parse_state = 4
                data_get[1] = ele
                data_to_check_crc.append(ele)
                print('to check:{0}'.format(chr(data_get[0]) + chr(data_get[1])))
                get_packet_flag = True
            if get_packet_flag == False:
                parse_state = 0
                data_to_check_crc = []
            else:
                parse_state = 4
            continue
                
        if parse_state == 4:
            if ele == 0:
                data_to_check_crc.append(ele)
                parse_state = 6
                continue
            elif ele <= 200:
                data_get[2] = ele
                data_to_check_crc.append(ele)
                parse_state = 5
                continue
            elif ele > 200:
                parse_state = 0
                data_to_check_crc = []
                continue
            continue
        if parse_state == 5:
            data_to_check_crc.append(ele)
            if (len(data_to_check_crc) - 3) == data_get[2]:
                parse_state = 6
                continue
            else:
                continue
        if parse_state == 6:
            crc_check[0] = ele
            parse_state = 7
            continue
        if parse_state == 7:
            crc_check[1] = ele
            parse_state = 0
            crc_get = calc_crc(data_to_check_crc)
            data_get = []
            if(crc_get[0] == crc_check[0]) and (crc_get[1] == crc_check[1]):
                print(bytes(data_to_check_crc))
            data_to_check_crc = []
    
    

if __name__=="__main__":
    python_serial = serial.Serial(com_set,int(baud_set),timeout=1)

    while True:
            user_cmd = input('cmd:\r\n')
            user_cmd_bytes = bytes(user_cmd, 'utf-8')
            gv_cmd = packet_cmd(user_cmd)
            data = python_serial.read_all()
            python_serial.write(gv_cmd)
            time.sleep(0.05)            
            while True:
                count = 0
                data = python_serial.read_all()
                if len(data) > 0:
                    print(data)
                    data_get = data_parse(data,user_cmd)
                    if data_get != None :
                        print(bytes(data_get))
                        break
                    else:
                        time.sleep(0.05)
                        print('sleep')
                        count+= 1
                        if count == 5:
                            break
                            
