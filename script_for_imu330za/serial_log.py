import os
import sys
import argparse
import time
from time import sleep
import datetime
import collections
import struct
import json
import serial

is_windows = sys.platform.__contains__(
    'win32') or sys.platform.__contains__('win64')
is_later_py_38 = sys.version_info > (3, 8)
is_later_py_3 = sys.version_info > (3, 0)

def receive_args():
    """parse input arguments
    """
    parser = argparse.ArgumentParser(
        description='Aceinna serial log args command:')
    
    parser.add_argument("-p", type=str, help="folder path", default='.')
    parser.add_argument("-c", type=str, help="serial name", default='ttyS1')
    parser.add_argument("-b", type=str, help="baudrate", default='115200')
    parser.add_argument("-t", type=str, help="type", default='A')    
    return parser.parse_args()

if __name__ == '__main__':
    # compatible code for windows python 3.8
    if is_windows and is_later_py_38:
        import asyncio
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

    args = receive_args()
    if args.c == '':
        print('Warning: use \'-c\' to set serial name.')
    else:
        ser = None
        outf = None

        try:
            ser = serial.Serial(args.c, args.b, timeout=0.005)
        except Exception as e:
            if ser is not None:
                if ser.isOpen():
                    ser.close()
            print(e)

        if ser and ser.isOpen():
            print('{0} {1} is open now'.format(args.c, args.b))
            fname = 'user_' + time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime()) + '.bin'
            print('log filename is {0}'.format(fname))
            fname = args.p + '/' + fname
            ser.write(bytes('COM COM3 460800\r\n',encoding = 'utf-8'))
            ser.close()
            ser = serial.Serial(args.c, args.b, timeout=0.005)
            
            file_type = 'wb'
            if args.t == 'A':
                file_type = 'w'
            with open(fname, file_type) as outf:
                length = 0
                count = 0
                while True:
                    try:
                        if args.t == 'A':
                            data = bytes.decode(ser.read_all())
                        else:
                            data = bytearray(ser.read_all())
                    except Exception as e:
                        if outf is not None:
                            outf.close()
                        print('SERIAL READ error:', e)
                        break
                    if len(data):
                        length = length + len(data)
                        outf.write(data)
                        count = count + 1
                        if count >= 10:
                            count = 0
                            #print(data)
                            sys.stdout.write("\rread length: %d" %(length))
                            sys.stdout.flush()
                    else:
                        time.sleep(0.001)
