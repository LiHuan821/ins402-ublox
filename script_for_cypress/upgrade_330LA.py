import os
import time
import sys

com_set = sys.argv[1]

while True:
    
    cmd = "jlink.exe {}".format("rtk_ins_upgrade.script")
    os.system(cmd)
    
    time.sleep(8)

    cmd = "TeseoProgrammer.exe program -f t5 -c {0} -b 230400 -m SQI -d 0x10000000 -+ 0x400 -e 3 -i sta9100_dbgoff_Bx_5_8_12_1_BETA_RTCM3_scn5_BOOT.BIN".format(com_set) 

    os.system(cmd)
    
    print('end')
    input('next xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx:')