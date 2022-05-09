'''
Author: y3s
LastEditors: y3s
email: y3sss@foxmail.com
Date: 2022-04-29 10:44:35
LastEditTime: 2022-05-01 11:57:47
motto: keep learning makes u strong
'''
import gmpy2
import binascii
p = 1254262028540861028019474719397
q = 1097836485532972216353973465081
e = 65537
c = 0x92a4545b2be1709f2df002fc540486b1d5ab5cb4d81cbd90f
n = p*q
f = (p-1)*(q-1)
d = int(gmpy2.invert(e,f))
m = pow(c,d,n)
print(binascii.unhexlify(hex(m)[2:].strip("L")))
print('明文:',m)
