import os
import json
import sys



def get_common_para(iar_path, hex_file):
    #if 'bootloader' in hex_file:
    common_para = "\"{0}/arm/bin/armproc.dll\" ".format(iar_path) +\
    "\"{0}/arm/bin/armJET.dll\" ".format(iar_path) +\
    "\"{0}\" ".format(hex_file) +\
    "--plugin=\"{0}/arm/bin/armbat.dll\" ".format(iar_path) +\
    "--device_macro=\"{0}/arm/config/debugger/Cypress/CYTVII_M0+.dmac\" ".format(iar_path) +\
    "--flash_loader=\"{0}/arm/config/flashloader/Cypress/FlashCYT4_CF8M_WF256K_CM0p.board\"".format(iar_path)
    return common_para


def get_core_para(iar_path, hex_file):
    core_para = \
    "\"--endian=little\" " +\
    "\"--cpu=Cortex-M0+\" " +\
    "\"--fpu=None\" " +\
    "\"-p\" " +\
    "\"{0}/arm\CONFIG\debugger\Cypress\CYT4BF_M0+.ddf\" ".format(iar_path) +\
    "\"--semihosting\" " +\
    "\"--device=CYT4BF_M0+\" " +\
    "\"--drv_mtb_regbase=0xF0003000\" " +\
    "\"--drv_mtb_minramaddr=0xF0010000\" " +\
    "\"--drv_mtb_maxramaddr=0xF0010FFF\" " +\
    "\"--multicore_nr_of_cores=1\" " +\
    "\"--jet_standard_reset=4,0,0\" " +\
    "\"--reset_style=\"0,-,0,Disabled__no_reset_\"\" " +\
    "\"--reset_style=\"1,-,0,Software\"" 
    "\"--reset_style=\"2,Traveo2_CM0p_HardwareReset,0,Hardware\"\" " +\
    "\"--reset_style=\"4,-,1,System\"\" " +\
    "\"--jet_power_from_probe=leave_on\" " +\
    "\"--drv_interface=SWD\" " +\
    "\"--drv_catch_exceptions=0xff0\" " +\
    "\"--jet_board_cfg={0}\arm\config\debugger\Cypress\CYT4.ProbeConfig\" ".format(iar_path) +\
    "\"--jet_board_did=M0+\" " +\
    "\"--board_file={0}\arm\config\flashloader\Cypress\FlashCYT4_CF8M_WF256K_CM0p.board\"".format(iar_path)    
    return core_para


def read_config(json_path):
    config_file_path = json_path
    json_dict = dict()
    if os.path.exists(config_file_path):
        fs = open(config_file_path,'r+')
        data = fs.read()
        if data != '':
            json_dict = json.loads(data)
    else:
        if not os.path.exists('./config'):
            os.makedirs('./config')
        fs = open(config_file_path,'w')            
    fs.close()
    return json_dict



if __name__ == '__main__':
    hex_file = os.path.join('../',os.path.basename(os.getcwd()),sys.argv[1])
    print('hex = {0}'.format(hex_file))
    new_path,file = os.path.split(sys.argv[0])
    os.chdir(new_path)
    config_dict = read_config('./config.json')
    iar_path = config_dict["iar_path"]
    
    common_para = get_common_para(iar_path, hex_file)
    core_para = get_core_para(iar_path, hex_file)

    cmd = "cspybat --download_only  {0} --backend {1}".format(common_para,core_para)
    os.system(cmd)
    
