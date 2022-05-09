<!--

 * @Author: y3s
 * @LastEditors: y3s
 * @email: y3sss@foxmail.com
 * @Date: 2022-05-02 10:55:04
 * @LastEditTime: 2022-05-02 10:58:40
 * @motto: keep learning makes u strong
-->

# 画图

开局一个pcap

根据题面结合wireshark可以知道是一个USB流量分析，那就写脚本做吧![1](E:\2022\2022校赛\misc-画图\题解\1.jpg)

首先看到又USB流量的数据包长度是14位，7个字节

并且他不是我们常见的leftover格式，所以在使用tshark提取时我们先保存为json文件看一下他的键是什么再提取![2](E:\2022\2022校赛\misc-画图\题解\2.jpg)

可以看到是叫usbhid.data

因此我们使用指令`tshark -r usb.pcap -T fields -e usbhid.data | sed '/^\s*$/d' > test2.txt`将数据提取出来![3](E:\2022\2022校赛\misc-画图\题解\3.jpg)

此时我们可以仔细看一下数据，不难发现第3-4位代表鼠标按键是否按下，5-6位代表x轴，7-8位代表y轴，因此我们就写一个脚本把这些数据转化成坐标

```python
'''
Author: y3s
LastEditors: y3s
email: y3sss@foxmail.com
Date: 2022-05-02 16:33:56
LastEditTime: 2022-05-02 17:02:46
motto: keep learning makes u strong
'''
#!/usr/bin/env python
# -*- coding:utf-8 -*-
nums = []
keys = open('test2.txt','r')
result=open('result.txt','w')
posx = 0
posy = 0
for line in keys:
    print(len(line))
    if len(line) != 15 :#忽略空行
        print('false')
        continue
    x = int(line[4:6],16)
    y = int(line[6:8],16)
    if x > 127 :
        x -= 256
    if y >115 :
        y -=266
    posx += x
    posy += y
    btn_flag = int(line[2:4],16)  # 1 for left , 2 for right , 0 for nothing
    if btn_flag == 1 :
        result.write(str(posx)+' '+str(-posy)+'\n')
keys.close()
result.close()
```

在使用gnuplot进行画图`gnuplot.exe -e "plot 'result.txt'" -p`



![4](E:\2022\2022校赛\misc-画图\题解\4.jpg)

即可得到最后的答案
