# 咩射频
433Mhz遥控信号与红外互转，完美兼容小爱音响红外学习，是最便宜的小爱音箱射频模块。 可以焊433Mhz与315Mhz模块二选一，使用蜂鸟无线最便宜的远-T1和微-R1

PCB已开源 https://oshwhub.com/zanjie1999/433-ir-converter

焊接视频

https://www.bilibili.com/video/BV1GG4y1S7RP

 

射频模块收发踩坑

https://www.bilibili.com/video/BV1YA41167zK

## 如何使用
在代码第13行附近可以选择开关功能，程序自带blink（点灯），使用RGB灯条的设备类型接入小爱音响（通过WiFi控制）可以修改程序让没有红外学习功能的小爱音响实现控制

sg90为舵机控制，实现开灯神器的功能

https://www.bilibili.com/video/BV1aG4y1u7Cn

433Mhz遥控信号与红外互转不需要联网也不需要配置，通电后就会自动将所有接收到的红外转成射频，射频转成红外，可以多做几个实现一个小爱万能遥控控制全家设备
