import os
import sys
import argparse
import time
from time import sleep
import datetime
import collections
import struct
import json
import math

is_windows = sys.platform.__contains__(
    'win32') or sys.platform.__contains__('win64')
is_later_py_38 = sys.version_info > (3, 8)
is_later_py_3 = sys.version_info > (3, 0)

class InceptioParse:
    def __init__(self, data_file, path, json_setting, inskml_rate):
        self.rawdata = []
        if is_later_py_3:
            self.rawdata = data_file.read()
        else:
            self.filedata = data_file.read()
            for c in self.filedata:
                self.rawdata.append(ord(c))
        self.path = path
        self.inskml_rate = 1/inskml_rate
        self.packet_buffer = []
        self.sync_state = 0
        self.sync_pattern = collections.deque(4*[0], 4)
        self.userPacketsTypeList = []
        self.log_files = {}
        self.f_imu = None
        self.gnssdata = []
        self.insdata = []
        self.pkfmt = {}
        self.last_time = 0

        with open(json_setting) as json_data:
            self.rtk_properties = json.load(json_data)

    def start_pasre(self):
        self.userPacketsTypeList = self.rtk_properties['userPacketsTypeList']
        for x in self.rtk_properties['userOutputPackets']:
            length = 0
            pack_fmt = '<'
            for value in x['payload']:
                if value['type'] == 'float':
                    pack_fmt += 'f'
                    length += 4
                elif value['type'] == 'uint32':
                    pack_fmt += 'I'
                    length += 4
                elif value['type'] == 'int32':
                    pack_fmt += 'i'
                    length += 4
                elif value['type'] == 'int16':
                    pack_fmt += 'h'
                    length += 2
                elif value['type'] == 'uint16':
                    pack_fmt += 'H'
                    length += 2
                elif value['type'] == 'double':
                    pack_fmt += 'd'
                    length += 8
                elif value['type'] == 'int64':
                    pack_fmt += 'q'
                    length += 8
                elif value['type'] == 'uint64':
                    pack_fmt += 'Q'
                    length += 8
                elif value['type'] == 'char':
                    pack_fmt += 'c'
                    length += 1
                elif value['type'] == 'uchar':
                    pack_fmt += 'B'
                    length += 1
                elif value['type'] == 'uint8':
                    pack_fmt += 'B'
                    length += 1
            len_fmt = '{0}B'.format(length)
            fmt_dic = collections.OrderedDict()
            fmt_dic['len'] = length
            fmt_dic['len_b'] = len_fmt
            fmt_dic['pack'] = pack_fmt
            self.pkfmt[x['name']] = fmt_dic

        packet_type = ''
        for i,new_byte in enumerate(self.rawdata):
            self.sync_pattern.append(new_byte)
            if self.sync_state == 1:
                self.packet_buffer.append(new_byte)
                if len(self.packet_buffer) == self.packet_buffer[2] + 5: # packet len
                    packet_crc = 256 * self.packet_buffer[-2] + self.packet_buffer[-1]
                    if packet_crc == self.calc_crc(self.packet_buffer[:-2]): # packet crc
                        self.parse_output_packet_payload(packet_type)
                        self.packet_buffer = []
                        self.sync_state = 0
                    else:
                        print('user data crc err!')
                        self.sync_state = 0  # CRC did not match
            else:
                for packet_type in self.userPacketsTypeList:
                    packet_type_0 = ord(packet_type[0])
                    packet_type_1 = ord(packet_type[1])
                    if list(self.sync_pattern) == [0x55, 0x55, packet_type_0, packet_type_1]: # packet type
                        self.packet_buffer = [packet_type_0, packet_type_1]
                        self.sync_state = 1
                        break

        self.close_files()

    def weeksecondstoutc(self,gpsweek,gpsseconds,leapseconds):
        import datetime, calendar
        datetimeformat = "%Y-%m-%d %H:%M:%S"
        epoch = datetime.datetime.strptime("1980-01-06 00:00:00",datetimeformat)
        elapsed = datetime.timedelta(days=(gpsweek*7),seconds=(gpsseconds+leapseconds))
        return datetime.datetime.strftime(epoch + elapsed,datetimeformat)

    def close_files(self):
        for i, (k, v) in enumerate(self.log_files.items()):
            v.close()

    def log(self, output, data):
        if output['name'] not in self.log_files.keys():
            self.log_files[output['name']] = open(self.path + output['name'] + '.csv', 'w')
            self.write_titlebar(self.log_files[output['name']], output)
        buffer = ''
        if output['name'] == 'S2':
            '''
            high = (data[0] >> 8) & 0xff;
            low = data[0] & 0xff;            
            week = low * 256 + high
            time_list = [(data[1] >> 24) & 0xff,(data[1] >> 16) & 0xff,(data[1] >> 8) & 0xff,(data[1] >> 0) & 0xff]
            print(time_list)
            time_now = (time_list[3] << 24) + (time_list[2] << 16) + (time_list[1] << 8) + time_list[0]
            print(time_now)
            '''
            buffer = buffer + format(data[0], output['payload'][0]['format']) + ","
            buffer = buffer + format(data[1] / 1000, output['payload'][1]['format']) + ","
            buffer = buffer + format(data[2], output['payload'][2]['format']) + ","
            buffer = buffer + format(data[3], output['payload'][3]['format']) + ","
            buffer = buffer + format(data[4], output['payload'][4]['format']) + ","
            buffer = buffer + format(data[5], output['payload'][5]['format']) + ","
            buffer = buffer + format(data[6], output['payload'][6]['format']) + ","
            buffer = buffer + format(data[7], output['payload'][7]['format']) + ","
            buffer = buffer + format(data[8], output['payload'][8]['format']) + ","
            status_hex = '{:#x}'.format(data[9])
            status = ' '*2 + status_hex[0:2] + '0'*(8+2-len(status_hex)) + status_hex[2:]
            
            buffer = buffer + format(status, output['payload'][9]['format']) + "\n"
        if output['name'] == 'GT':
            buffer = buffer + format(data[0] / 1000, output['payload'][0]['format']) + ","
            buffer = buffer + format(data[1] , output['payload'][1]['format']) + "\n"

        self.log_files[output['name']].write(buffer)            
    def parse_output_packet_payload(self, packet_type):
        payload_lenth = self.packet_buffer[2]
        payload = self.packet_buffer[3:payload_lenth+3]
        output = next((x for x in self.rtk_properties['userOutputPackets'] if x['name'] == packet_type), None)
        if output != None:
            self.openrtk_unpack_output_packet(output, payload, payload_lenth)
        else:
            print('no packet type {0} in json'.format(packet_type))

    def openrtk_unpack_output_packet(self, output, payload, payload_lenth):
        fmt = self.pkfmt[output['name']]
        len_fmt = fmt['len_b']
        pack_fmt = fmt['pack']
        if output['isList']:
            length = fmt['len']
            packet_num = payload_lenth // length
            for i in range(packet_num):
                payload_c = payload[i*length:(i+1)*length]
                try:
                    b = struct.pack(len_fmt, *payload_c)
                    data = struct.unpack(pack_fmt, b)
                    self.log(output, data)
                except Exception as e:
                    print("error happened when decode the {0} {1}".format(output['name'], e))
        else:
            try:
                b = struct.pack(len_fmt, *payload)
                data = struct.unpack(pack_fmt, b)
                self.log(output, data)
            except Exception as e:
                print("error happened when decode the {0} {1}".format(output['name'], e))

    def write_titlebar(self, file, output):
        for value in output['payload']:
            file.write(value['name']+'('+value['unit']+')')
            file.write(",")
        file.write("\n")

    def calc_crc(self, payload):
        crc = 0x1D0F
        for bytedata in payload:
            crc = crc^(bytedata << 8)
            for i in range(0,8):
                if crc & 0x8000:
                    crc = (crc << 1)^0x1021
                else:
                    crc = crc << 1

        crc = crc & 0xffff
        return crc


def mkdir(file_path):
    path = file_path.strip()
    path = path.rstrip("\\")
    path = path[:-4]
    path = path + '_p'
    if not os.path.exists(path):
        os.makedirs(path)
    return path

def receive_args():
    parser = argparse.ArgumentParser()
    parser.description = argparse.ArgumentParser(
        description='Aceinna OpenRTK python parse input args command:')
    parser.add_argument("-p", type=str, help="folder path", default='.')
    parser.add_argument("-i", type=int, help="ins kml rate(hz): 1 2 5 10", default=5)
    return parser.parse_args()

if __name__ == '__main__':
    # compatible code for windows python 3.8
    if is_windows and is_later_py_38:
        import asyncio
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

    args = receive_args()


    json_setting = 'imu330za_packets.json'

    if args.i != 1 and args.i != 2 and args.i != 5 and args.i != 10:
        print('waring: no ins kml rate {0}, just can be 1 2 5 10!'.format(args.i))
        sys.exit(0)

    for root, dirs, file_name in os.walk(args.p):
        for fname in file_name:
            if fname.startswith('user') and fname.endswith('.bin'):
                file_path = os.path.join(root, fname)
                print('processing {0}'.format(file_path))
                path = mkdir(file_path)
                try:
                    with open(file_path, 'rb') as fp_rawdata:
                        if fname.startswith('user'):
                            parse = InceptioParse(fp_rawdata, path + '/' + fname[:-4] + '_', json_setting, args.i)
                        parse.start_pasre()
                        fp_rawdata.close
                except Exception as e:
                    print(e)
