echo start
cd "C://Program Files (x86)//SEGGER//JLink"
tasklist /fi "Imagename eq JLinkGDBServer.exe"|find "JLinkGDBServer.exe"&&taskkill /f /im "JLinkGDBServer.exe"
tasklist /fi "Imagename eq JLinkGDBServer.exe"|find "JLinkGDBServer.exe"&&taskkill /f /im "JLinkGDBServer.exe"
start JLinkGDBServer.exe -select USB -device %1 -if JTAG -speed 1000 -noir

