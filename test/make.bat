set ROOT_PATH=%~dp0
set PRJ_PATH=%ROOT_PATH%RTK330LA_BF_m7_0\

echo %ROOT_PATH%
echo %PRJ_PATH%

echo pause

cd %PRJ_PATH%
cd build && make all -j8

set PRJ_PATH=%ROOT_PATH%RTK330LA_BF_m7_1\
cd %PRJ_PATH%
cd build && make all -j8


timeout /t 15
