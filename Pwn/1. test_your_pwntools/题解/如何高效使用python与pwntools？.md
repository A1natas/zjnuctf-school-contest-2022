# 【题解】如何高效使用python与pwntools

## 前言

该题属于测试选手对于python的使用和pwntools的api使用的简单题，选手需要会基本使用python的基础api与pwntools常用的api！

## 解题步骤

我们首先nc测试一下题目所给的ip地址

![image-20220504163425549](https://cdn.jsdelivr.net/gh/Awoodsheep/pic/imgs/image-20220504163425549.png)

根据英文意思，我们只需要在30秒内计算100次正确答案就可以获取flag

难点在于，30手算是基本不可能完成的任务，所以需要借助python与pwntools的使用

我们可以注意到，题目给出了`symbol`符号，还有两个`nums`，并且给出了两次

但是分析，其实我们可以仅仅接收最后一行的一个整式，使用`recvuntil("answer?")`，就可以将前面的垃圾数据进行接收处理

处理字符串方式的计算式，可以使用python中的`eval()`函数，直接得到一个int型数据，需要注意，题目中乘法是使用`x`来做符号的，需要`replace('x','*')`来处理一下！

当然，也可以使用自己的if-else语句来进行判断

最终的解题脚本如下：

```python
from pwn import *

# connect
io = remote("101.34.90.86",19198)

# recv useless info
io.recvuntil("Let's have a try!\n")

# 100 times
for i in range(100):
	# recv one line with valueable infos, and decoded by utf-8
	io.recvuntil("So what's the answer? ")
	line = io.recvline()
	line = str(line,encoding="utf-8")
	log.info("line --> " + line)

	# if you use funcation eval(), you just need do this!
	# io.sendline(str(eval(str(line.split("=")[0]).replace("x","*"))))

	# you can alse use this way to handle infos!
	infos = line.split(" ")
	
	# we needed infos
	num1 = int(infos[0])
	symbol = infos[1]
	num2 = int(infos[2])

	# calculation
	res = 0
	if symbol == "+":
		res = num1 + num2
	elif symbol == "-":
		res = num1 - num2
	elif symbol == 'x':
		res = num1 * num2
	else:
		res = num1 // num2
	res = str(res)
	log.info("ans --> " + res)
	
	# send ans
	io.sendline(res)

# interactive
io.interactive()

```

注意，该python的版本属于3.8，如果使用python2，其中的pwntools没有识别`byte`类型，每次recv接收的都是`str`类型，无需python3中如此麻烦进行转化

运行后，结果如下：

![image-20220504163717282](https://cdn.jsdelivr.net/gh/Awoodsheep/pic/imgs/image-20220504163717282.png)

flag{Y@u_h4ve_kn0wn_pwntools}