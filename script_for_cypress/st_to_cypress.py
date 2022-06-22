import serial
import binascii
import time
import socket   
from time import sleep
import base64
import _thread
import datetime
import json
import os
import sys
import argparse

host = '58.215.20.43'
#host = 'rtk.ntrip.qxwz.com'
port = 2201
#port = 8002
#mountPoint = 'RTKBASE'
mountPoint = 'WX02'
#mountPoint = 'RTCM32_GGB'
userAgent = 'NTRIP Aceinna CloudRTK 1.0'
username = 'ymj_123'
password = 'SIGEMZOOMQ1JDJI3'




def receive_args():
    """parse input arguments
    """
    parser = argparse.ArgumentParser(
        description='Aceinna serial log args command:')
    
    parser.add_argument("-c", type=str, help="serial name", default='com28')
    parser.add_argument("-b", type=str, help="baudrate", default='460800')
    return parser.parse_args()

def recv(serial):
	while True:
		if select == "1":
			data = serial.read_all()
			#print('get data')
		else:
			data = binascii.b2a_hex(serial.read_all())
		if data == '':
			continue
		else:
			break
		#sleep(0.02)
	return data

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
	#return utc_st


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



file_time = time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime())
#mk_time =   time.strftime("%Y_%m_%d",time.localtime())
day = get_utc_day()
mkpath='./' + day
mkdir(mkpath)
log_file = mkpath + '/' + 'debug_' + file_time
print (log_file)



def rev_ntrip_data(client,serial_base,is_log):

	if is_log:
		day = get_utc_day()
		try:
			mkdir(day)
		except:
			pass
		file_time = time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime())
		log_file = day + '/' + 'base_' + file_time +'.bin'
		fs = open(log_file,'wb')
	while True:
		try:
			rev_data = client.recv(1024)
			if(len(rev_data) > 0):
				#print('len = %s' % (len(rev_data)))
				serial_base.write(rev_data)
				#port_handle.write(rev_data)
				if is_log:
					fs.write(rev_data)
				#print(rev_data)
			sleep(0.01)
		except:
			time.sleep(3)
			print('rev none')
			break
def ntip_connect(serial_base):
    try:
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
            #input('wait')
            print ('connect ntrip suc start connect com')
            _thread.start_new_thread(rev_ntrip_data,(rtk_client,serial_base,1,))
            return rtk_client
    except:
        pass


def uart_cypress_rev(serial_user_base):
    file_time = time.strftime("%Y_%m_%d_%H_%M_%S", time.localtime())
    log_file = day + '/' + 'user_' + file_time +'.bin'
    fs = open(log_file,'wb')
    length = 0
    while True:
        data_user = serial_user_base.read_all()
        if(len(data_user) > 0):
            length = length + len(data_user)
            sys.stdout.write("\rread user length: %d" %(length))
            sys.stdout.flush()
            fs.write(data_user)

def rover_uart_connect(serial_user_base):
	if serial_user_base.isOpen() :
		print("open success")
	else :
		print("open failed")
	_thread.start_new_thread(uart_cypress_rev,(serial_user_base,))



if __name__ == '__main__':
    args = receive_args()
    serial_base = serial.Serial(args.c, args.b, timeout=0)

    base_client = ntip_connect(serial_base)
    rover_uart_connect(serial_base)

    sleep(3)
    count = 0
    while True:
        sleep(1)
        try:
            base_client.send(bytes('hello base','utf-8'))
        except Exception as e:
            print(e)
            print('send fail')
            count+= 1
            sleep(1)
            if count == 5:
                count = 0
                while True:
                    print('reconnect')
                    base_client = ntip_connect(serial_base)
                    sleep(2)
                    if base_client != None:
                        print('reconnect suc')
                        break
                    
