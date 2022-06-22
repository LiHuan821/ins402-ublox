# -*- coding: utf-8 -*- 
import os
import time

def fileCntIn(currPath):
    return sum([len(files) for root, dirs, files in os.walk(currPath)])

def dirsTree(startPath):
    fs = open('ewp_test.ewp','w+')
    start_flag = 0
    include_path_list = []
    old_level = 0
    last_level = 0
    for root, dirs, files in os.walk(startPath):
        if('.git' in root) or ('Obj' in root) or ('../../../iar' in root) or ('script' in root):
            continue
        if(root == startPath):
            continue
        include_path_list.append('                    <state>$PROJ_DIR$\\..\\..\\' + root.split('..')[-1].replace('/','\\')[1:] + '</state>')
        #print(str(include_path_list[0]))
        fileCount = fileCntIn(root)
        level = root.replace(startPath, '').count(os.sep)
        last_level = level
        level_sub = old_level - level
        if level_sub > 0:
            #print(level_sub)
            for i in range(level_sub):
                group_str = '    ' * 1 * (level - i + 1) + "</group>" + '\n'
                fs.write(group_str)            
        indent = '| ' * 1 * level + '|____'
        old_level = level

        if(level == 0):
            start_flag = 1
            group_str = '    <group>' + '\n'
            fs.write(group_str) 
            group_str = '    ' * 1 * (level+2) + "<name>" + os.path.split(root)[1] + '</name>' + '\n'
            fs.write(group_str)    
        if(level == 1): 
            group_str = '    ' * 1 * (level+1) + "<group>" + '\n'
            fs.write(group_str)
            group_str = '    ' * 1 * (level+2) + '<name>' + os.path.split(root)[1] + '</name>' + '\n'
            fs.write(group_str)   
        if(level >= 2):
            group_str = '    ' * 1 * (level+1) + "<group>" + '\n'
            fs.write(group_str)
            group_str =  '    ' * 1 * (level+2) + '<name>' + os.path.split(root)[1] + '</name>' + '\n'
            fs.write(group_str)   
        #print(group_str)
        
        for file in files:
            str_test = '    ' * 1 * (level+2) + '<file>' + '\n'
            fs.write(str_test)   
            str_test = '    ' * 1 * (level+3) + '<name>' +\
            '$PROJ_DIR$\\' + '..\\' * 1 * (2) + root.split('..')[-1].replace('/','\\')[1:] + '\\' + file + '</name>' + '\n' 
            fs.write(str_test)             
            str_test = '    ' * 1 * (level+2) + '</file>' + '\n'
            fs.write(str_test) 
    
        if(len(dirs) == 0):   
            str_test = '    ' * 1 * (level + 1) + "</group>" + '\n'
            fs.write(str_test)     
        #print(root)
        #print(root.split('..')[-1].replace('/','\\'))
        #input('wait\r\n')    
        '''
        print ('%s%s fileCount:%s' % (indent, os.path.split(root)[1], fileCount))
        '''
    for i in range(last_level):
        group_str = '    ' * 1 * (last_level - i) + "</group>" + '\n'
        fs.write(group_str) 
    fs.close()
    fs_file = open('ewp_test.ewp','r')
    fs_ori1 = open('ori1.ewp','r')
    fs_ori2 = open('ori2.ewp','r')
    fs_now = open('rtk_ins.ewp','w+')
    data = fs_ori1.read()
    for ele in include_path_list:
        data+= str(ele)
        data+= '\n'
    data += fs_ori2.read()
    data += fs_file.read()
    data += '\n</project>'
    fs_now.write(data)

if __name__ == '__main__':
    start_time = time.time()
    path = u"../../../"
    dirsTree(path)
    end_time = time.time()
    print ("time use:%.2fs" % (end_time - start_time))
