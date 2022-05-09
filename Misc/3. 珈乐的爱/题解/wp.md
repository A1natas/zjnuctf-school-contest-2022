<!--
 * @Author: y3s
 * @LastEditors: y3s
 * @email: y3sss@foxmail.com
 * @Date: 2022-05-02 10:55:04
 * @LastEditTime: 2022-05-02 10:58:40
 * @motto: keep learning makes u strong
-->

# 珈乐的爱
1、根据提示，知道是一个电话音识别，找到DTMF工具，进行爆破，得到压缩包密码

![1](E:\2022\2022校赛\misc-珈乐的爱\题解\1.png)

压缩包解压之后，发现里面有一张图片，根据图片信息可以知道是一个LSB

![2](E:\2022\2022校赛\misc-珈乐的爱\题解\2.png)

用stegsolve打开可以看到红绿蓝的0通道都有奇怪的东西，可以确定是LSB，使用提取工具可以提出一张png图片

![3](E:\2022\2022校赛\misc-珈乐的爱\题解\3.png)

可以看到只有半张图片，因此我们选择调整他的高度，就能找到flag了

![flag](E:\2022\2022校赛\misc-珈乐的爱\题解\flag.png)

