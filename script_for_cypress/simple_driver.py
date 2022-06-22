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
    
            
        
        
            
    
    
    

if __name__=="__main__":
    python_serial = serial.Serial(com_set,int(baud_set),timeout=1)

    while True:
            user_cmd = input('cmd:\r\n')
            user_cmd_bytes = bytes(user_cmd, 'utf-8')
            gv_cmd = packet_cmd(user_cmd)
            data = python_serial.read_all()
            python_serial.write(gv_cmd)
            time.sleep(0.1)
            while True:
                data = python_serial.read_all()
                if len(data) > 0:
                    data_get = data_parse(data,user_cmd)
                    if data_get != None :
                        print(bytes(data_get))
                    break
