'''
Author: y3s
LastEditors: y3s
email: y3sss@foxmail.com
Date: 2022-05-04 14:24:56
LastEditTime: 2022-05-04 14:29:16
motto: keep learning makes u strong
'''
'''
Author: y3s
LastEditors: y3s
email: y3sss@foxmail.com
Date: 2022-05-04 14:19:17
LastEditTime: 2022-05-04 14:24:02
motto: keep learning makes u strong
'''
a = ['\u202d','\u202e']

FILEPATH = r'E:\\2022\\2022校赛\\nothingencode\\题目附件\\flag.txt'

def nothing_encode(flag):
    binstr = ""
    for i in flag:
        nowbinstr = bin(ord(i))[2:]
        binstr = binstr + '0'*(16-len(nowbinstr)) + nowbinstr
    result = ''
    with open(FILEPATH,'w',encoding='utf-8') as f:
        for i in binstr:
            f.write(a[int(i)])
            result += a[int(i)]
    return result.encode('utf-8')

def nothing_decode():
    with open(FILEPATH,'r',encoding='utf-8') as f:
        cipertext = f.read()
    cipertext = cipertext.replace('\u202d','0').replace('\u202e','1')
    flag = ''
    for i in range(0,len(cipertext),16):
        flag += chr(int(cipertext[i:i+16],2))
    return flag

# print(nothing_encode(flag))
print(nothing_decode())

