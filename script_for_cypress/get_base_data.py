import socket   
from time import sleep
import base64
import _thread
import serial
import time
import datetime
import os
import argparse
import sys


host = '58.215.20.43'
#host = 'rtk.ntrip.qxwz.com'
port = 2201
#port = 8002
#mountPoint = 'RTKBASE'
mountPoint = 'WX02'
#mountPoint = 'RTCM32_GGB'
userAgent = 'NTRIP Aceinna CloudRTK 1.0'
username = 'yundong'
password = 'TEL8IOZTBJVVJ0IT'


def receive_args():
    """parse input arguments
    """
    parser = argparse.ArgumentParser(
        description='Aceinna serial log args command:')
    
    parser.add_argument("-c", type=str, help="serial name", default='com28')
    parser.add_argument("-b", type=str, help="baudrate", default='460800')
    return parser.parse_args()


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
	
def mkdir(path):
    path=path.strip()
    path=path.rstrip("\\")
    isExists=os.path.exists(path)
    if not isExists:
        os.makedirs(path)
        print (path+' mkdir suc')
        return True
    else:
        print ('mkdir exist')
        return False

def rev_ntrip_data(client,port_handle,is_log):
	if is_log:
		day = get_utc_day()
		try:
			mkdir(day)
		except:
			pass
		file_time = time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime())
		log_file = day + '/' + 'ntrip_' + file_time +'.bin'
		fs = open(log_file,'wb')
	while True:
		rev_data = client.recv(1024)
		if(len(rev_data) > 0):
			#print('len = %s' % (len(rev_data)))
			port_handle.write(rev_data)
			if is_log:
				fs.write(rev_data)
			#print(rev_data)
		sleep(0.1)
def rev_uart_data(port_handle,client,is_log):
    length = 0
    if is_log:
        day = get_utc_day()
        try:
            mkdir(day)
        except:
            pass
        file_time = time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime())
        log_file = day + '/' + 'user_' + file_time +'.bin'
        fs = open(log_file,'wb')
    while True:
        data = port_handle.read_all()
        if len(data) > 0 :
            length = length + len(data)
            sys.stdout.write("\rread user length: %d" %(length))
            sys.stdout.flush()
            #print(length)
            if is_log:
                fs.write(data)
        sleep(0.01)
        
def ntip_connect(args):
    uart_handle = None
    ntrip_run = True

    try:
        uart_handle = serial.Serial(args.c, args.b, timeout=0.005)
    except Exception as e:
        if uart_handle is not None:
            if uart_handle.isOpen():
                uart_handle.close()
        print(e)
    #uart_handle=serial.Serial(com_port,460800, timeout=1)
    if(uart_handle.is_open):
        print('wrong port')

    rtk_client = socket.socket()  
    rtk_client.connect((host, port))
    auth = username + ':' + password
    bytes_auth = auth.encode("utf-8")
    authorization = base64.b64encode(bytes_auth)
    #authorization = username + ':' + password
    info = "GET /%s HTTP/1.0\r\nUser-Agent: %s\r\nAuthorization: Basic %s\r\n\r\n"%(mountPoint,userAgent,authorization.decode('utf-8'))
    print ("info = %s" % info)

    rtk_client.send(info.encode("utf8"))
    rev_data = rtk_client.recv(1024)
    if('ICY 200 OK' in str(rev_data)):
        print ('connect ntrip suc start connect com')
        _thread.start_new_thread(rev_ntrip_data,(rtk_client,uart_handle,1,))
        _thread.start_new_thread(rev_uart_data,(uart_handle,rtk_client,1,))
        return rtk_client,uart_handle
    '''
    try:
        serial=serial.Serial(com_port,460800, timeout=1)
        if(serial == NULL):
            print('wrong port')
        rtk_client = socket.socket()  
        rtk_client.connect((host, port))
        auth = username + ':' + password
        bytes_auth = auth.encode("utf-8")
        authorization = base64.b64encode(bytes_auth)
        #authorization = username + ':' + password
        info = "GET /%s HTTP/1.0\r\nUser-Agent: %s\r\nAuthorization: Basic %s\r\n\r\n"%(mountPoint,userAgent,authorization.decode('utf-8'))
        print (info)
        
        rtk_client.send(info.encode("utf8"))
        rev_data = rtk_client.recv(1024)
        if('ICY 200 OK' in str(rev_data)):
            print ('connect ntrip suc start connect com')
            _thread.start_new_thread(rev_ntrip_data,(serial,rtk_client,0,))
            _thread.start_new_thread(rev_uart_data,(serial,rtk_client,0,))
    except:
        print('error')
        return
    '''



if __name__ == '__main__':
    args = receive_args()
    base_client,uart_handle = ntip_connect(args)
    while True:
        sleep(5)
        try:
            base_client.send(bytes('hello base','utf-8'))
        except:
            print('reconnect')
            sleep(5)
            uart_handle.close()
            base_client = ntip_connect(args)
        pass
