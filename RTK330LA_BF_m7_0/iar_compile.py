# -*- coding:utf-8 -*-
import xml.etree.ElementTree as ET
import time
import getpass
import os
import subprocess


def runcmd(command):
    ret = subprocess.run(command,shell=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE,encoding="utf-8",timeout=1)
    return ret

def update_compile_message():
    compile_time = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())
    compile_author = getpass.getuser()
    git_log = runcmd('git log')
    compile_commit = git_log.stdout.split('\n', 1)[0][41:47]

    tree = ET.parse('./iar_8.4/rtk.custom_argvars')
    root = tree.getroot()
    for node_root in list(root):
        para_type = None
        for node in list(node_root):
            for node_para in list(node):
                if node_para.text == "COMPILE_TIME":
                    para_type = 'time'
                    continue
                elif node_para.text == "COMPILE_AUTHOR":
                    para_type = 'author'
                    continue
                elif node_para.text == "APP_VERSION":
                    para_type = 'version'
                    continue
                elif node_para.text == "GIT_COMMIT":
                    para_type = 'commit'
                    continue
                if(para_type == 'time'):
                    node_para.text = compile_time
                elif(para_type == 'author'):
                    node_para.text = compile_author
                elif(para_type == 'commit'):
                    node_para.text = compile_commit
    tree.write('./iar_8.4/rtk.custom_argvars', encoding='utf-8', xml_declaration=True)

if __name__ == "__main__":
    update_compile_message()
    cmd = "IarBuild.exe ./iar_8.4/rtk_m7_0/rtk.ewp -build Debug -parallel 4 -varfile ./iar_8.4/rtk.custom_argvars"
    os.system(cmd)

