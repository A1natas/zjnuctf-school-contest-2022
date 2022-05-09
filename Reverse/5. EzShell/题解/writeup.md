## this is a writeup
````
➜  ct cp myaes myaes_unpack
➜  ct upx -d myaes_unpack 
                       Ultimate Packer for eXecutables
                          Copyright (C) 1996 - 2018
UPX 3.95        Markus Oberhumer, Laszlo Molnar & John Reiser   Aug 26th 2018

        File size         Ratio      Format      Name
   --------------------   ------   -----------   -----------
     31648 <-     15672   49.52%   linux/amd64   myaes_unpack

Unpacked 1 file.
➜  ct ./myaes_unpack flag{5124E49125F0EC5811E33D2BF14711E2}
fail
➜  ct ./myaes flag{5124E49125F0EC5811E33D2BF14711E2}
success

```

算法是aes算法。
主要考点就是upx带壳运行和upx脱掉后运行内存还是有差别的，如果仅对脱壳后的程序拿不到真正的aes密钥。


solve.py
```
from Crypto.Cipher import AES
from zio import *

key = ''.join(chr(i) for i in range(32))
key = l64(0xf30000000000841f)+l64(0xf3000000c3fa1e0f) +l64(0x4808ec8348fa1e0f)+l64(0xc35a050fc308c483)
iv = ''.join(chr(i) for i in range(16))
cryptos = AES.new(key, AES.MODE_ECB)
m = '9ea72be8de91ea83fecc1b243b9736282dc9865f884e09c8b01bc8fe237627d5'.decode('hex')
print 'flag{'+cryptos.encrypt(m)+'}'
```

flag为flag{5124E49125F0EC5811E33D2BF14711E2}