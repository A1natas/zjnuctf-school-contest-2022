from Crypto.Cipher import AES
from zio import *

key = ''.join(chr(i) for i in range(32))
key = l64(0xf30000000000841f)+l64(0xf3000000c3fa1e0f) +l64(0x4808ec8348fa1e0f)+l64(0xc35a050fc308c483)
iv = ''.join(chr(i) for i in range(16))
cryptos = AES.new(key, AES.MODE_ECB)
m = '9ea72be8de91ea83fecc1b243b9736282dc9865f884e09c8b01bc8fe237627d5'.decode('hex')
print 'flag{'+cryptos.encrypt(m)+'}'