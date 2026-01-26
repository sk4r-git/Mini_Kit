from pwn import *
import subprocess

TARGET = b"rw"


def set_target():
    io.sendlineafter(b"q : pour quitter", b"a")
    res = io.recvuntil(b"Choisir une option")
    l = b""
    res = res.splitlines()
    for _ in res:
        if (b"Nom: " + TARGET) in _:
            l = _
    pid = l.split(b" ")[1]
    pid = int(pid[:-1])
    


io = process("./user/cli")


io.interactive()

