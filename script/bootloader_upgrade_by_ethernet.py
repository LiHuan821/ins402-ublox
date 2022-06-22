import psutil
from psutil import net_if_addrs, net_if_stats
from scapy.all import conf, sendp, sniff,srp
from scapy.layers.l2 import Ether
import struct
import os
import time

def get_list_from_int(value):
    value_list = [0 for x in range(0, 4)]
    value_list[3] = value & 0xff
    value_list[2] = (value >> 8) & 0xff
    value_list[1] = (value >> 16) & 0xff
    value_list[0] = (value >> 24) & 0xff
    return value_list

class ethernet_upgrade():
    '''Ethernet'''

    def __init__(self, options=None):
        super().__init__()
        self.type = '100base'
        self.src_mac = '6C:2B:59:5F:3E:AE'  #conf.iface.mac
        print(self.src_mac)
        self.dst_mac = '04:00:00:00:00:04'  # TODO: predefined or configured?
        self.ethernet_name = None
        self.data = None
        self.iface = '以太网'

        self.filter_device_type = None
        self.filter_device_type_assigned = False
        self.command_start = [0x55, 0x55]
        self.pG = [0x01, 0xcc]
        self.JI = [0x01, 0xaa]
        self.JA = [0x02, 0xaa]
        self.WA = [0x03, 0xaa]
        self.CS = [0x04, 0xaa]
        if options and options.device_type != 'auto':
            self.filter_device_type = options.device_type
            self.filter_device_type_assigned = True

    def find_device(self, callback, retries=0, not_found_handler=None):
        self.device = None
        
        # find network connection
        ifaces_list = self.get_network_card()

        command_line =b"\x04\x00\x00\x00\x00\x04tx'xH\xa4\x00\x00UU\x01\xcc\x00\x00\x00\x00\xcc\r\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        filter_exp = 'ether src host ' + self.dst_mac
        for i in range(len(ifaces_list)):
            ans = srp(Ether( _pkt=command_line, src="FF:FF:FF:FF:FF:FF", dst=self.dst_mac), timeout=1, iface = ifaces_list[i][0], filter=filter_exp, retry = 2,  verbose = 0)
            print(self.src_mac,self.dst_mac)
            if ans[0]:
                print(ans)
                self.iface = ifaces_list[i][0]
                self.src_mac = ifaces_list[i][1].replace('-', ':')
                break
            else:
                if i == len(ifaces_list) - 1:
                    print('The available Ethernet connection was not found.')
        # confirm device
        print('test')
        self.confirm_device(self)
        if self.device:
            callback(self.device)

    def calc_crc(self,payload):
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
        
        
    def build_packet(self, dest, src, message_type, message_bytes=[]):
        '''
        Build final packet
        '''
        whole_packet=[]
        src_bytes = bytes([int(x, 16) for x in src.split(':')])
        dest_bytes = bytes([int(x, 16) for x in dest.split(':')])
        print(src_bytes,dest_bytes)
        header = dest_bytes + src_bytes + bytes([ 0, 0])
        #header = dest + src + bytes([ 0, 0])
        
        whole_packet.extend(header)

        packet = []
        packet.extend(message_type)
        msg_len = len(message_bytes)

        packet_len = struct.pack("<I", msg_len)
        print(msg_len,packet_len)        
        packet.extend(packet_len)
        print('packet_len = ',packet_len)
        final_packet = packet + message_bytes

        whole_packet.extend(self.command_start)
        whole_packet.extend(final_packet)
        whole_packet.extend(self.calc_crc(final_packet))
        packet_hex = []
        for ele in whole_packet:
            packet_hex.append(hex(ele))
        print(packet_hex)
        
        return bytes(whole_packet)

    def write(self, data, is_flush=False):
        '''
        write
        '''
        try:
            sendp(data, iface=self.iface, verbose = 0)
            print(data)
        except Exception as e:
            raise

    def read(self,size=0):
        '''
        read
        '''
        filter_exp = 'ether src host ' + self.dst_mac
        data = sniff(count = 1, iface = self.iface, filter = filter_exp, timeout = 1)
        if data:
            # print('answer', data[0].original)
            return data[0].original
        return None

    def write_read(self, data, timeout=1):
        filter_exp = 'ether src host ' + self.dst_mac
        ans = srp(Ether(_pkt=data), iface = self.iface, filter = filter_exp, timeout = timeout, retry = 3, verbose = 0)
        try:
            if ans[0].res[0].answer:
                return bytes(ans[0].res[0].answer)
        except:
            return None

    def run_command(self, command, message=[], timeout=1):
        command_line = self.build_packet(
            self.dst_mac, self.src_mac, command, message)
        #print('command_line = ',command_line)
        data_buffer = []
        #print('line:',command_line)
        read_line = self.write_read(command_line,timeout)
        print('read_line:',read_line)
        get_msg_id = False
        if read_line:
            packet_raw = read_line[14:]
            packet_type = packet_raw[2:4]

            if packet_type == bytes(command):
                packet_length = struct.unpack('<i', packet_raw[4:8])[0]
                data_buffer = packet_raw[8: 8 + packet_length]
                get_msg_id = True
        print('data_buffer = {0}'.format(data_buffer))
        return (data_buffer,get_msg_id)

    def run_pG_command(self):
        return self.run_command(self.pG)
    def run_JI_command(self):
        return self.run_command(self.JI)
    def run_JA_command(self):
        return self.run_command(self.JA)
    def run_WA_command(self,data_to_write,packet_data_len, write_len):
        print(packet_data_len, write_len)
        message = []
        addr_3 = (write_len & 0xFF000000) >> 24
        addr_2 = (write_len & 0x00FF0000) >> 16
        addr_1 = (write_len & 0x0000FF00) >> 8
        addr_0 = (write_len & 0x000000FF)
        message.insert(len(message), addr_3)
        message.insert(len(message), addr_2)
        message.insert(len(message), addr_1)
        message.insert(len(message), addr_0)
        data_len_remain = packet_data_len % 16
        data_len_ori = packet_data_len
        if(data_len_remain != 0):
            packet_data_len = packet_data_len + (16 - data_len_remain)
        packet_data_len_list = get_list_from_int(packet_data_len)
        #message.insert(len(message), packet_data_len_list)
        message+= packet_data_len_list
        for i in range(data_len_ori):
            message.insert(len(message), data_to_write[i])
        if(data_len_remain != 0):
            message[4] = packet_data_len + 5
            for i in range(16 - data_len_remain):
                message.insert(len(message), 0xff)
        '''
        crc = calc_crc(message[2:message[4]+5])  
        crc_msb = int((crc & 0xFF00) >> 8)
        crc_lsb = int((crc & 0x00FF))
        message.insert(len(message), crc_msb)
        message.insert(len(message), crc_lsb)
        '''
        cmd = self.WA
        ret_buff,ret_id = self.run_command(cmd,message,timeout=20)
        if write_len == 0:
            print('sleep start')
            time.sleep(12)
        else:
            #sleep(0.01)
            time.sleep(0.01)
        return(ret_buff,ret_id)

    def run_set_core_command(self,size,core):
        size_list = get_list_from_int(size)
        cmd = self.CS 
        message = [ord('C'), ord(core)] + size_list
        print(message)
        return self.run_command(cmd,message)
        
    def reset_buffer(self):
        '''
        reset buffer
        '''
        pass

    def get_src_mac(self):    
        return bytes([int(x, 16) for x in self.src_mac.split(':')])

    def get_dst_mac(self):
        return bytes([int(x, 16) for x in self.dst_mac.split(':')])
    
    def get_network_card(self):
        network_card_info = []
        info = net_if_addrs()

        for k, v in info.items():
            for item in v:
                if item[0] == -1 and not item[1] == '127.0.0.1':
                    network_card_info.append((k, item[1]))
        return network_card_info

max_data_len = 192


if __name__ == '__main__':
    boot_tool = ethernet_upgrade()
    count = 0
    bin_file = 'rtk.bin'
    binfile = open(bin_file, 'rb')
    core = '0'
    size = os.path.getsize(bin_file) 
    fs_len = size
    write_len = 0
    
    _,get_id = boot_tool.run_JI_command()
    if get_id != None and get_id == True:
        print('JI suc')
    else:
        print('JI fail')
        exit(-1)
    time.sleep(8)
    _,get_id = boot_tool.run_set_core_command(size,core)
    if get_id != None and get_id == True:
        print('CS suc')
    else:
        print('CS fail')
        exit(-1)
    
    while (write_len < fs_len):
        data_to_write = binfile.read(max_data_len)
        if (fs_len - write_len) > max_data_len:
            packet_data_len = max_data_len 
        else:
            packet_data_len = fs_len - write_len
        boot_tool.run_WA_command(data_to_write,packet_data_len, write_len)
        #input('wait')
        write_len += packet_data_len
        print ("upload progress: %.3f%%" % (float(write_len)/float(fs_len)*100))
    print ("upload progress: %.3f%%" % 100)    
    _,get_id = boot_tool.run_JA_command()
    if get_id != None and get_id == True:
        print('JA suc')
    else:
        print('JA fail')
        exit(-1)    
    
    time.sleep(5)
    bin_file = 'ins.bin'
    binfile = open(bin_file, 'rb')
    core = '1'
    size = os.path.getsize(bin_file) 
    fs_len = size
    write_len = 0 
    _,get_id = boot_tool.run_JI_command()
    if get_id != None and get_id == True:
        print('JI suc')
    else:
        print('JI fail')
        exit(-1)
    time.sleep(8)
    _,get_id = boot_tool.run_set_core_command(size,core)
    if get_id != None and get_id == True:
        print('CS suc')
    else:
        print('CS fail')
        exit(-1)
    
    while (write_len < fs_len):
        data_to_write = binfile.read(max_data_len)
        if (fs_len - write_len) > max_data_len:
            packet_data_len = max_data_len 
        else:
            packet_data_len = fs_len - write_len
        boot_tool.run_WA_command(data_to_write,packet_data_len, write_len)
        #input('wait')
        write_len += packet_data_len
        print ("upload progress: %.3f%%" % (float(write_len)/float(fs_len)*100))
    print ("upload progress: %.3f%%" % 100)

   
    
    _,get_id = boot_tool.run_JA_command()
    if get_id != None and get_id == True:
        print('JA suc')
    else:
        print('JA fail')
        exit(-1)
    
'''
if __name__ == '__main__':
    boot_tool = ethernet_upgrade()
    count = 0
    bin_file = 'rtk.bin'
    binfile = open(bin_file, 'rb')
    core = '0'
    size = os.path.getsize(bin_file) 
    fs_len = size
    while True:
        count+= 1
        if count > 2:
            break
        else:
            #ret = boot_tool.run_JI_command()
            _,get_id = boot_tool.run_JA_command()
            #_,get_id = boot_tool.run_set_core_command(size,core)
            if get_id != None and get_id == True:
                print(get_id)
                break
    print('JI suc')
    time.sleep(6)
    write_len = 0
    bin_file = 'rtk.bin'
    binfile = open(bin_file, 'rb')
    core = '0'
    size = os.path.getsize(bin_file) 
    fs_len = size    
    ethernet_upgrade.run_set_core_command(size,core)
'''
        
        
        