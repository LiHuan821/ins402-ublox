/*******************************************************************************
*                            (C) 2018 STMicroelectronics
*    Reproduction and Communication of this document is strictly prohibited
*      unless specifically authorized in writing by STMicroelectronics.
*-----------------------------------------------------------------------------
*                                  APG / CRM / SA&PD
*                   Software Development Group - SW platform & HW Specific
******************************************************************************/

/*****************************************************************************
includes
*****************************************************************************/
#include "tp_common.h"
#include "tp_help.h"

/*****************************************************************************
external declarations
*****************************************************************************/

/*****************************************************************************
defines and macros (scope: module-local)
*****************************************************************************/

/*****************************************************************************
typedefs and structures (scope: module-local)
*****************************************************************************/

/*****************************************************************************
global variable definitions  (scope: module-exported)
*****************************************************************************/
const tChar html_header[] = "\n<!DOCTYPE html>\n<html>\n<head>\n<style>\n#customers{\nfont-family: Arial,Helvetica,sans-serif;;\nborder-collapse: collapse;\nwidth: 100 % ;\n}\n#customers td, #customers th{\nborder: 1px solid #ddd;\npadding: 8px;\n}\n#customers tr : nth-child(even) { background-color: #f2f2f2; }\n#customers tr : hover{ background-color: #ddd; }\n#customers th{\npadding-top: 12px;\npadding-bottom: 12px;\ntext-align: left;\nbackground-color: #39A9DB;\ncolor: white;\n}\nh1\n{\nmargin-left: 40px;\nbackground-color: #39A9DB;\n}\nh2\n{\nmargin-left: 80px;\n}\nh3\n{\nmargin-left: 120px;\nfont-style: oblique;\n}\np\n{\nmargin-left: 60px;\nfont-family: Arial,Helvetica,sans-serif;;\n}\nli{list-style-type: square;\nmargin-left: 80px;\nfont-family: Arial, Helvetica, sans-serif;;}\n</style>\n</head>\n<body>\n";

const tChar html_table_begin[] = "<table id=\"customers\">\n<tr><th>Usage</th><th>Description</th><th>Type/Choice</th><th>Sample</th></tr>\n";
const tChar html_table_end[] = "</table>\n";

const tChar html_logo[] = "\n<img style='display:block; width:100px;height:100px;'id='base64image' src='data:image/jpeg;base64,\
iVBORw0KGgoAAAANSUhEUgAAAEwAAAAoCAMAAABjLOxVAAACZ1BMVEX////EzNTn6/BXgaxYhLBa\
hrNLa4tSeaA5YII4XoEvS2T+/v5WgKrt8PNdi7lcirg3W3zFzdZciLVYg65Wf6hTeqI1V3ZZhbFV\
fadUfKVRd50/bZU9apA6Y4dqlb5QdZs5YYVkkbs+a5I9aI9HZII6XX02WXr7/P3d5e1nkrxhjrpf\
jblAcJpAbpdPcpVKaYlJZ4U7X4A4XX83XH45W3oyUW3h6vFMbpE8Z407ZYo1WHgzUm/g6fGApsZ2\
nsJij7tbibdah7VPc5dLbY9JaIc0VHH3+Prf5+57o8RznMFvmcB4nr5slbo7ZIk8YIFFYH00VXQx\
T2vg6O+0ytxXeplOcZRHZoQwTmn9/f74+vvp7vJsl79Ygq1Le6dafJtSdpkvTGf09vje5u7d4+jQ\
2OCxyNqswdN9pMV4ocNzmr1plLppkbZehKdTeZ5UeJrk6e2uxNaqv9B2nLxxmLlei7Zkia1TfKNO\
eaNCc59Bcp1cfptWd5ZNcJNOcJJQcI5CYHzv8/fq7/XU4OvG1ubU3OSfvNWzwcxvmLxnj7NkjLJ9\
l61VfKNBcJtMbI08YoRGYoD8/P3Bz9zL09uvxtnGz9fAytOGqMiesMCRpbhulLaGnLFag6xgh6tV\
f6p0j6hZgadwi6RAcZxcfZpifphogZdfeZBHaYpAW3fm7fTf5eq/0+O6zuCrwtmmu865xM2Nrcuv\
vcqjt8mrucWessSZr8N9oMKMqcGCosGYq72bqriGn7aNnq9mh6VZfqFGdaFSdJZWcYtObYhDZYZB\
Y4Oat9K1ws+SsM92lLFrjKl8k6V7jqB0iZ1dfJbYRcBrAAAEzklEQVRIx62V919SURiHbzHUJDEk\
SUXANMtEAqkQSzNISwNpCLnQzFEiYmmu3OXWLFNzj5ztvffef1TvuZxLdbXBR5/fLvfe537fc973\
QCzE+dQNv3AEsZviAGY7xVYHWQuosu9/tlqtIyMjpW1tba2tBQUFgzpdSUlJfnHxXori4nygpaVF\
B1gs8Exr6+C1eaqsgaRMX5fVPnxOOpPF43ps3BEQIhKlJcTHRx/bs+doVFTU0T3HoqPjExLSRKKQ\
gB2hHlwei8ngrPEpTaW7Vk7XaMSkjPFvWcivso6ndFfKtFaGZZz/km10yGy9NJf7E1O1XVYmVmm1\
WlV1dbVRJnv1qqqqKikpKWNy8hQiA0gCbLZMhEYjMxprZtbTZPVBqhokE8++fna7b9nfWfU72QSN\
u7tUNUaNWDXT604sluQptrbGKFO1xxGLJ4fN1qqMxjGofvE8JWWZqUvhStVsYp9Vye4TS8ENLZKJ\
Ty+Fa32pfBfUOb4e58w7AeQBx4GenlzgoJ3L+IXm/nt3bpzIO557sIG++2fE8l2b2Fod3tkJm62w\
o4M/PjwsYPFOhoaiUQgJsYhEopIjBEnz+EmAx2MJSrbSk12Um0B2Nh8fQ69l5Fit4TO8YK5iuB6g\
2xEQEBAS0I7HZToUZilmp4Dp9ZAe7LI1KAhF0+EbfQZs4zC8BNhG6l40EyQXNB7IxWJ6WbvpY5lb\
e46UlTYQ+GG5DA2pD5+07QQb6AA5PgS/MrlcHnIxntObPKXYcC7IJN/EzrhKYBprNfZCOajSnSgc\
+DjP8AapuDE8VCOjo3/e8lcZDPY6Hzi+cyVD/NOGwsVwuR4yPNF3OBBLwExncGqL6FVGGQw4mq3J\
8aHlz30LsS2dKRCgdKwxqpFABbEY/NXfCBqnxXN1VLQXTcmOg3fQhbJBOEjHGu3GlQhABbHWlLve\
nNcX3lti5+ogGrIVfnSMetYDH7CVYR342vCtW0g1Wv6yUmqinwsNwkP7t2yL9d5cF4R6TfwoxbEx\
t4Zh3SAcqWMIruMXLKNm88uKyktS4WN6sHuxSLYtdp93pKfi3dvZ6k9Zjil7b/V1gXBIx+dY6gmS\
HnNFBag6pWF+qfSxNK04DDZSFuypDHdz9Z9cSWDiLhb6+roA5eXl/Cac11JRWdkplYaFSR/Rg908\
TMocNjc3V9c3RY6uMQvXrvXzgxelXvhIuWYGE6j8/MzzDpmZFSvs0SgbyNzqsqnTPDQwEOnAl4a7\
RgQiMMGP0fSGzd4GMkehyBYONte6Rnz/YWAE6MDXiZe/8RKYQSUUll2lBxvYT8mwDVf67gpBclui\
VpM6IV7+9rVIJAwMjNDRxzLLhFx0G4Tr6scHV7C/HnQRki+4ldeBB0wR6rAmerBVis0U60gUCkUi\
YhbXeVcp8ffX69WROfbrJ24RgFqt15u302XJy/8A1blDbq4S0PlP4effqJFIDx9oIZzlfG04WkFJ\
1wf7dW+XP0ICH+h2WtangBUEHxsv9pgEAdut/J5MP3z+5YqrCg729FQqFQP266IuaEKEci7b2Vzu\
7cHekZHgS8Qj8VgRjlCGB+U4/W86tHkfNAt0ywQeS7YniWIC5M5VmTOViPvlLc5xIRF1Tu0QXDpL\
/UqKOOrr5IX7QiF+AK3AgCUMFNn9AAAAAElFTkSuQmCC\
'/>";

const tChar html_footer[] = "</body></html>";

/*****************************************************************************
global variable definitions (scope: module-local)
*****************************************************************************/

/*****************************************************************************
function prototypes (scope: module-local)
*****************************************************************************/

/*****************************************************************************
function implementations (scope: module-local)
*****************************************************************************/

