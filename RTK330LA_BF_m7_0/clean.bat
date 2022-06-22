set ROOT_PATH=%~dp0
set PRJ_PATH=%ROOT_PATH%iar_8.4\

echo %ROOT_PATH%
echo %PRJ_PATH%

echo pause

cd %PRJ_PATH%
cd rtk_m7_0\Debug\Exe
DEL /F/S/Q *.sim
DEL /F/S/Q *.hex
DEL /F/S/Q *.bin

cd %PRJ_PATH%
cd rtk_m7_0\Debug\List
DEL /F/S/Q *.map

cd %PRJ_PATH%
cd rtk_m7_0\Debug\Obj
DEL /F/S/Q	*.o
DEL /F/S/Q	*.dep
DEL /F/S/Q	*.pbi
DEL /F/S/Q	*.xcl
DEL /F/S/Q  *.pbd
DEL /F/S/Q  *.pbw
DEL /F/S/Q  *.browse

timeout /t 15
